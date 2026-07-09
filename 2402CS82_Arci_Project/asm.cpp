// 2401CS82 Deepanshu Parte

#include <bits/stdc++.h>
#include <iomanip>
using namespace std;

// Symbol and Literal Records
struct SymRec
{
    string lbl;
    int address;
    bool setFlag;   
    bool used;      
    int setVal;
};

struct LitRec
{
    int lit_val;
    int address;
};

vector<SymRec> symtab;
vector<LitRec> littab;

// ---------- Utility Functions ----------

string strip(string s)
{
    if (s.empty()) return s;
    size_t st = s.find_first_not_of(" \t\n\r");
    size_t en = s.find_last_not_of(" \t\n\r");
    if (st == string::npos) return "";
    return s.substr(st, en - st + 1);
}

bool isNum(const string &s)
{
    if (s.empty()) return false;
    const char *p = s.c_str();
    char *end;
    strtol(p, &end, 0);
    return (end != p && *end == '\0');
}

int toInt(const string &s)
{
    return (int)strtol(s.c_str(), nullptr, 0);
}

bool symLookup(string name)
{
    for (auto &i : symtab)
        if (i.lbl == name)
            return true;
    return false;
}

string hex32(int x)
{
    stringstream ss;
    ss << setfill('0') << setw(8) << hex << uppercase << (unsigned int)x;
    return ss.str();
}

int chkLabel(const string &label)
{
    if (label.empty() || !isalpha((unsigned char)label[0]))
        return 0;
    for (char c : label)
        if (!isalnum((unsigned char)c) && c != '_')
            return 0;
    return 1;
}

// ---------- Opcode Table ----------
map<string, int> optab;

void initOptab()
{
    optab["ldc"]    = 0x00;
    optab["adc"]    = 0x01;
    optab["ldl"]    = 0x02;
    optab["stl"]    = 0x03;
    optab["ldnl"]   = 0x04;
    optab["stnl"]   = 0x05;
    optab["add"]    = 0x06;
    optab["sub"]    = 0x07;
    optab["shl"]    = 0x08;
    optab["shr"]    = 0x09;
    optab["adj"]    = 0x0a;
    optab["a2sp"]   = 0x0b;
    optab["sp2a"]   = 0x0c;
    optab["call"]   = 0x0d;
    optab["return"] = 0x0e;
    optab["brz"]    = 0x0f;
    optab["brlz"]   = 0x10;
    optab["br"]     = 0x11;
    optab["HALT"]   = 0x12;
    optab["data"]   = 0x13;  
    optab["SET"]    = 0x14;  
}

bool isNoOperand(const string &op)
{
    return op == "add" || op == "sub" || op == "shl" || op == "shr" ||
           op == "a2sp" || op == "sp2a" || op == "return" || op == "HALT";
}

bool isBranch(const string &op)
{
    return op == "brz" || op == "brlz" || op == "br" || op == "call";
}

bool emitsWord(const string &op)
{
    return !op.empty() && op != "SET" && optab.count(op);
}

// ---------- PASS 1 ----------
string p1_parse(const string &instr, int &lc, int line)
{
    string err;

    if (instr.find(':') != string::npos)
    {
        size_t pos   = instr.find(':');
        string label = strip(instr.substr(0, pos));

        if (symLookup(label))
            err += "ERROR (line " + to_string(line) + "): Duplicate label '" + label + "'\n";
        else if (!chkLabel(label))
            err += "ERROR (line " + to_string(line) + "): Invalid label name '" + label + "'\n";
        else
            symtab.push_back({label, lc, false, false, -1});

        string rest = (pos + 1 < instr.size()) ? strip(instr.substr(pos + 1)) : "";

        if (rest.empty())
            return err;   

        if (rest.size() >= 3 && rest.substr(0, 3) == "SET" &&
            (rest.size() == 3 || isspace((unsigned char)rest[3])))
        {
            string setOpnd = strip(rest.substr(3));
            if (isNum(setOpnd) && !symtab.empty())
            {
                symtab.back().setFlag   = true;
                symtab.back().setVal    = toInt(setOpnd);
                symtab.back().address   = toInt(setOpnd);
            }
            else
            {
                err += "ERROR (line " + to_string(line) + "): SET requires a numeric operand\n";
            }
            return err;   
        }

        
        err += p1_parse(rest, lc, line);
        return err;
    }

    size_t sp  = instr.find(' ');
    string op  = (sp != string::npos) ? instr.substr(0, sp) : instr;
    string opnd = (sp != string::npos) ? strip(instr.substr(sp + 1)) : "";

    if (op == "SET")
    {
        err += "WARNING (line " + to_string(line) + "): SET without a label has no effect\n";
        return err;
    }

    if (!op.empty() && !optab.count(op))
        err += "ERROR (line " + to_string(line) + "): Unknown mnemonic '" + op + "'\n";

    if (emitsWord(op))
        lc++;

    return err;
}

void pass1(const string &file, ofstream &logf)
{
    logf << "------------- PASS 1 STARTED -------------\n";
    ifstream in(file);
    string line;
    int lc = 0, lno = 1;

    while (getline(in, line))
    {
        size_t semi  = line.find(';');
        string instr = strip(semi != string::npos ? line.substr(0, semi) : line);

        if (instr.empty()) { lno++; continue; }

        string err = p1_parse(instr, lc, lno);
        if (!err.empty()) logf << err;
        lno++;
    }

    logf << "\nSymbol Table:\n";
    for (auto &s : symtab)
        logf << left << setw(20) << s.lbl << s.address
             << (s.setFlag ? "  [SET]" : "") << "\n";
    logf << "\n------------- PASS 1 FINISHED -------------\n\n";
    in.close();
}

// ---------- PASS 2 — Instruction Encoding ----------
tuple<string,string,string> encodeInst(const string &instr, int lc, int line)
{
    string err, mc;

    if (instr.find(':') != string::npos)
    {
        size_t pos  = instr.find(':');
        string rest = (pos + 1 < instr.size()) ? strip(instr.substr(pos + 1)) : "";

        if (rest.empty())
            return {"SKIP", err, mc};  

        // Check for SET after label
        if (rest.size() >= 3 && rest.substr(0, 3) == "SET" &&
            (rest.size() == 3 || isspace((unsigned char)rest[3])))
            return {"SKIP", err, mc};

        return encodeInst(rest, lc, line);
    }

    size_t sp   = instr.find(' ');
    string op   = (sp == string::npos) ? instr : instr.substr(0, sp);
    string opnd = (sp == string::npos) ? ""    : strip(instr.substr(sp + 1));

    if (op == "SET")
        return {"SKIP", err, mc};

    if (!optab.count(op))
    {
        err += "ERROR (line " + to_string(line) + "): Unknown mnemonic '" + op + "'\n";
        return {"", err, mc};
    }

    int opcode = optab[op];

    if (op == "data")
    {
        if (opnd.empty())
        {
            err += "ERROR (line " + to_string(line) + "): 'data' requires an operand\n";
            return {"", err, mc};
        }
        int32_t val = 0;
        if (isNum(opnd))
        {
            val = toInt(opnd);
        }
        else
        {
            bool found = false;
            for (auto &s : symtab)
                if (s.lbl == opnd) { val = s.address; found = true; break; }
            if (!found)
                err += "ERROR (line " + to_string(line) + "): Undefined symbol '" + opnd + "'\n";
        }
        mc = hex32(val);
        return {"", err, mc};
    }

    // --- no-operand instructions ---
    if (isNoOperand(op))
    {
        if (!opnd.empty())
            err += "WARNING (line " + to_string(line) + "): '" + op +
                   "' takes no operand, '" + opnd + "' ignored\n";
        mc = hex32((int)(uint32_t)opcode);
        return {"", err, mc};
    }

    // --- operand required ---
    if (opnd.empty())
    {
        err += "ERROR (line " + to_string(line) + "): '" + op + "' requires an operand\n";
        return {"", err, mc};
    }

    // Check for extra tokens after operand
    {
        size_t extra = opnd.find_first_of(" \t,");
        if (extra != string::npos)
        {
            err += "ERROR (line " + to_string(line) + "): Extra text after operand: '" +
                   opnd.substr(extra) + "'\n";
            opnd = opnd.substr(0, extra);
        }
    }

    int32_t operandVal = 0;

    if (isNum(opnd))
    {
        operandVal = toInt(opnd);
    }
    else
    {
        bool found = false;
        for (auto &s : symtab)
        {
            if (s.lbl == opnd)
            {
                found   = true;
                s.used  = true;   
                if (isBranch(op))
                    operandVal = s.address - (lc + 1);  
                else
                    operandVal = s.address;             
                break;
            }
        }
        if (!found)
        {
            err += "ERROR (line " + to_string(line) + "): Undefined symbol '" + opnd + "'\n";
            return {"", err, mc};
        }
    }

    uint32_t word = ((uint32_t)(int32_t)operandVal << 8) | (uint32_t)opcode;
    mc = hex32((int)word);
    return {"", err, mc};
}

// ---------- PASS 2 ----------

void pass2(const string &file, ofstream &lst, ofstream &logf, ofstream &obj)
{
    logf << "------------- PASS 2 STARTED -------------\n";
    ifstream in(file);
    string line;
    int lc = 0, lno = 1;

    lst << "Line    Address         MachineCode     Source\n";
    lst << "------------------------------------------------\n";

    while (getline(in, line))
    {
        size_t semi  = line.find(';');
        string instr = strip(semi != string::npos ? line.substr(0, semi) : line);
        if (instr.empty()) { lno++; continue; }

        // FIX C: capture address BEFORE encoding
        string addrStr = hex32(lc);

        string enc, err, mc;
        tie(enc, err, mc) = encodeInst(instr, lc, lno);

        // No address shown for lines that emit no word
        if (enc == "SKIP" || mc.empty())
            addrStr = "";

        lst << left
            << setw(8)  << lno
            << setw(16) << addrStr
            << setw(16) << mc
            << instr << "\n";


        if (!err.empty())
            logf << err;

        if (!mc.empty())
        {
            uint32_t val = (uint32_t)stoul(mc, nullptr, 16);
            obj.write((char*)&val, sizeof(val));
            lc++;   
        }
        lno++;
    }

    // Warn about labels defined but never used
    for (auto &s : symtab)
        if (!s.used && !s.setFlag)
            logf << "WARNING: Label '" << s.lbl << "' defined but never referenced\n";

    in.close();
    logf << "\n------------- PASS 2 FINISHED -------------\n";
}

// ---------- Main ----------

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: ./asm file.asm\n";
        return 1;
    }

    initOptab();

    string infile = argv[1];
    string base   = infile.substr(0, infile.find_last_of('.'));

    ofstream lst(base + ".lst");
    ofstream logf(base + ".log");
    ofstream obj(base + ".bin", ios::binary);

    if (!logf) { cerr << "Error opening log file\n";     return 1; }
    if (!lst)  { cerr << "Error opening listing file\n"; return 1; }
    if (!obj)  { cerr << "Error opening binary file\n";  return 1; }

    logf << "Assembler Started\n\n";

    pass1(infile, logf);
    pass2(infile, lst, logf, obj);

    logf << "\nAssembler Finished\n";
    lst.close(); logf.close(); obj.close();

    cout << "Done. Output: " << base << ".bin  " << base << ".lst  " << base << ".log\n";
    return 0;
}