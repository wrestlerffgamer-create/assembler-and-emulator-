// 2401CS82 Deepanshu Parte
#include <bits/stdc++.h>
using namespace std;

// Constants
#define MEM_SIZE 10000

// CPU Registers 
int32_t A, B, PC, SP;

// Main Memory 
int32_t MainMem[MEM_SIZE];

// Instruction descriptor 
struct Instr {
    string mnemonic;
    bool   hasOperand;
};

// Opcode Table 
map<int, Instr> Op_Table;

// Function Prototypes 
string toHex8(int32_t value);
void   initOpTable();
pair<int32_t,int32_t> decode(int32_t instruction);
void   showUsage();
void   showISA();
void   memDump(int length, ofstream &out, ofstream *log = nullptr);
int    execute(int length, bool trace, ofstream &log, ofstream &out);

// --- Convert 32-bit integer to 8-digit hex ---
string toHex8(int32_t value)
{
    stringstream ss;
    ss << setfill('0') << setw(8) << hex << uppercase << (uint32_t)value;
    return ss.str();
}

// --- Decode instruction ---
pair<int32_t,int32_t> decode(int32_t instruction)
{
    int32_t opcode  = instruction & 0xFF;
    int32_t operand = instruction >> 8;

    if (operand & 0x800000)
        operand |= 0xFF000000;
    return make_pair(operand, opcode);
}

// --- Initialize opcode table ---         
void initOpTable()
{
    Op_Table[0]  = {"ldc",    true};
    Op_Table[1]  = {"adc",    true};
    Op_Table[2]  = {"ldl",    true};
    Op_Table[3]  = {"stl",    true};
    Op_Table[4]  = {"ldnl",   true};
    Op_Table[5]  = {"stnl",   true};
    Op_Table[6]  = {"add",    false};
    Op_Table[7]  = {"sub",    false};
    Op_Table[8]  = {"shl",    false};
    Op_Table[9]  = {"shr",    false};
    Op_Table[10] = {"adj",    true};
    Op_Table[11] = {"a2sp",   false};
    Op_Table[12] = {"sp2a",   false};
    Op_Table[13] = {"call",   true};
    Op_Table[14] = {"return", false};
    Op_Table[15] = {"brz",    true};
    Op_Table[16] = {"brlz",   true};
    Op_Table[17] = {"br",     true};
    Op_Table[18] = {"HALT",   false};
}

// --- Show usage ---
void showUsage()
{
    cout << "Usage: ./emu [option] file.o\n"
         << "Options:\n"
         << "  -trace   show instruction trace\n"
         << "  -before  memory dump before execution\n"
         << "  -after   memory dump after execution\n"
         << "  -isa     display ISA\n";
}

// --- Show ISA ---
void showISA()
{
    cout << "Opcode  Mnemonic  Operand\n"
         << "-------------------------\n"
         << " 0      ldc       value\n"
         << " 1      adc       value\n"
         << " 2      ldl       offset\n"
         << " 3      stl       offset\n"
         << " 4      ldnl      offset\n"
         << " 5      stnl      offset\n"
         << " 6      add\n"
         << " 7      sub\n"
         << " 8      shl\n"
         << " 9      shr\n"
         << "10      adj       value\n"
         << "11      a2sp\n"
         << "12      sp2a\n"
         << "13      call      offset\n"
         << "14      return\n"
         << "15      brz       offset\n"
         << "16      brlz      offset\n"
         << "17      br        offset\n"
         << "18      HALT\n";
}

// --- Memory Dump ---
void memDump(int length, ofstream &out, ofstream *log)
{
    auto writeLine = [&](const string &line) {
        out << line << "\n";
        if (log) *log << line << "\n";
    };

    writeLine("Memory Dump");
    writeLine("----------------------------------------------------");
    writeLine("Address   Value      Instruction");
    writeLine("----------------------------------------------------");

    for (int i = 0; i < length; i++)
    {
        int32_t word = MainMem[i];
        pair<int32_t,int32_t> decoded = decode(word);
        int32_t operand = decoded.first;
        int32_t opcode  = decoded.second;

        string line = toHex8(i) + "  " + toHex8(word) + "  ";

        
            line += to_string(word);
        

        writeLine(line);
    }
    writeLine("----------------------------------------------------");
}
    
// --- Execute Program ---
int execute(int length, bool trace, ofstream &log, ofstream &out)
{
    int  count   = 0;
    bool errorOccurred = false;

    if (trace)
        log << "---- Execution Trace ------------------------------------------\n"
            << "PC        SP        A         B         Instruction\n"
            << "---------------------------------------------------------------\n";

    while (true)
    {
        if (PC < 0 || PC >= length)
        {
            log << "ERROR: PC=" << toHex8(PC)
                << " is out of bounds (program length=" << length << ")\n";
            cerr << "ERROR: PC out of bounds at " << PC << "\n";
            errorOccurred = true;
            break;
        }

        int32_t word = MainMem[PC];
        pair<int32_t,int32_t> decoded = decode(word);
        int32_t operand = decoded.first;
        int32_t opcode  = decoded.second;

        if (!Op_Table.count(opcode))
            log << "WARNING: Unknown opcode " << opcode
                << " encountered at PC=" << toHex8(PC) << "\n";

        if (trace)
        {
            string instrStr = Op_Table.count(opcode)
                              ? Op_Table[opcode].mnemonic : "???";
            if (Op_Table.count(opcode) && Op_Table[opcode].hasOperand)
                instrStr += " " + to_string(operand);

            string line = toHex8(PC) + "  " + toHex8(SP) + "  "
                        + toHex8(A)  + "  " + toHex8(B)  + "  " + instrStr + "\n";
            log  << line;
            cout << line;
        }

        PC++;

        switch (opcode)
        {
            case 0:  B = A; A = operand; break;
            case 1:  A += operand; break;
            case 2:  B = A; A = MainMem[SP + operand]; break;
            case 3:  MainMem[SP + operand] = A; A = B; break;
            case 4:  A = MainMem[A + operand]; break;
            case 5:  MainMem[A + operand] = B; break;
            case 6:  A = B + A; break;
            case 7:  A = B - A; break;
            case 8:  A = B << A; break;
            case 9:  A = (int32_t)((uint32_t)B >> (uint32_t)A); break;
            case 10: SP += operand; break;
            case 11: SP = A; A = B; break;
            case 12: B = A; A = SP; break;
            case 13: B = A; A = PC; PC += operand; break;
            case 14: PC = A; A = B; break;
            case 15: if (A == 0) PC += operand; break;
            case 16: if (A < 0)  PC += operand; break;
            case 17: PC += operand; break;
            case 18:
                log << "\n---- HALT --------------------------------------------\n"
                    << "Instructions executed : " << count << "\n"
                    << "\nFinal Register State:\n"
                    << "  A  = " << setw(12) << (int)A
                    << "  (0x" << toHex8(A)  << ")\n"
                    << "  B  = " << setw(12) << (int)B
                    << "  (0x" << toHex8(B)  << ")\n"
                    << "  PC = " << setw(12) << (int)PC
                    << "  (0x" << toHex8(PC) << ")\n"
                    << "  SP = " << setw(12) << (int)SP
                    << "  (0x" << toHex8(SP) << ")\n";

                cout << count << " instructions executed\n";
                memDump(length, out, &log);   
                return 0;

            default:
                log << "ERROR: Illegal opcode " << opcode
                    << " at PC=" << toHex8(PC - 1) << "\n";
                cerr << "ERROR: Illegal opcode " << opcode << "\n";
                errorOccurred = true;
                break;
        }

        if (errorOccurred) break;
        count++;
    }

    log << "Execution stopped after " << count << " instructions due to error.\n";
    memDump(length, out, &log);
    return 1;
}

// --- Main ---
int main(int argc, char* argv[])
{
    if (argc < 2) { showUsage(); return 1; }
    if (string(argv[1]) == "-isa") { showISA(); return 0; }
    if (argc < 3) { showUsage(); return 1; }

    initOpTable();

    string option    = string(argv[1]);
    string inputFile = string(argv[2]);

    string base    = inputFile.substr(0, inputFile.rfind('.'));
    string logName = base + "_emu.log"; 
    string outName = base + ".out";     

    ifstream objFile(inputFile);
    if (!objFile.is_open())
    {
        cerr << "ERROR: Cannot open input file '" << inputFile << "'\n";
        return 1;
    }

    ofstream logFile(logName);
    ofstream outFile(outName);
    if (!logFile.is_open() || !outFile.is_open())
    {
        cerr << "ERROR: Cannot create log/out file\n";
        return 1;
    }

    logFile << "SIMPLEX Emulator Log\n"
            << "Input file : " << inputFile << "\n"
            << "============================================\n\n";

    int32_t word;
    int     length = 0;

while (objFile.read((char*)&word, sizeof(word)))
{ 
    if (length >= MEM_SIZE)
    {
        logFile << "ERROR: Program exceeds memory limit\n";
        cerr << "ERROR: Program too large\n";
        return 1;
    }
    MainMem[length++] = word;   // load as-is, no special casing
}
    objFile.close();

    logFile << "Program loaded : " << length << " words\n\n";
    cout << "Loaded " << length << " words from '" << inputFile << "'\n";

    // Initialize CPU
    A  = 0; B = 0; PC = 0; SP = 0;

    // Execute based on option
    if (option == "-before")
    {
        logFile << "---- Memory Dump BEFORE Execution ---------------\n"
                << "(full dump in " << outName << ")\n\n";
        memDump(length, outFile, &logFile);
        execute(length, false, logFile, outFile);
    }
    else if (option == "-trace")
    {
        execute(length, true, logFile, outFile);
    }
    else if (option == "-after")
    {
        execute(length, false, logFile, outFile);
        logFile << "\n---- Memory Dump AFTER Execution ----------------\n"
                << "(full dump in " << outName << ")\n";
        memDump(length, outFile, &logFile);
    }
    else
    {
        logFile << "WARNING: Unknown option '" << option << "' - no execution performed.\n";
        cerr << "WARNING: Unknown option '" << option << "'\n";
        showUsage();
        return 1;
    }

    logFile.close();
    outFile.close();

    cout << "Log  written to : " << logName << "\n" 
         << "Dump written to : " << outName << "\n";

    return 0;
}