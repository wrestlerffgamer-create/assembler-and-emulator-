;BUBBLE SORT
ldc 0x1000
a2sp          ; Initialize Stack Pointer to 0x1000
call sort
HALT

sort:
stl 5     ; Save return address to stack at SP[5]

ldc count
ldnl 0    
adc -1    
stl 0     ; SP[0] = n - 1 (Outer loop counter)


outer_loop:
ldl 0
brz end_sort  ; If outer counter hits 0, array is sorted

ldc 0
stl 1     ; SP[1] = 0 (Inner loop counter 'i')


inner_loop:
ldl 1
ldl 0
sub       ; A = outer - inner
brz end_inner ; If inner index == outer limit, inner loop done

; Load address of array[i]
ldc array
ldl 1
add       
stl 2     ; SP[2] = pointer to array[i]

; Load values at array[i] and array[i+1]
ldl 2
ldnl 0    
stl 3     ; SP[3] = val at array[i]

ldl 2
adc 1     ; A = pointer to array[i+1]
ldnl 0    ; A = value at array[i+1]
stl 4     ; SP[4] = val at array[i+1]

; Compare array[i] and array[i+1]
ldl 4     ; A = array[i+1]
ldl 3     ; A = array[i], B = array[i+1]
sub       ; A = array[i+1] - array[i]
brlz swap ; If array[i+1] < array[i], branch to swap
br no_swap


swap:
; Perform Memory Swap: array[i] = array[i+1]
; stnl requires: A = Address, B = Value
ldl 4     ; A = value of array[i+1]
ldl 2     ; B = val array[i+1], A = ptr array[i]
stnl 0

; array[i+1] = array[i]
ldl 3     ; A = value of array[i]
ldl 2
adc 1     ; B = val array[i], A = ptr array[i+1]
stnl 0    


no_swap:
ldl 1
adc 1
stl 1     ; i++
br inner_loop

end_inner:
ldl 0
adc -1
stl 0     ; outer--
br outer_loop

end_sort:
ldl 5     ; Restore return address into A
return

; --- Data Section ---
array:  data 5
data 2
data 9
data 1
data 6
count:  data 5