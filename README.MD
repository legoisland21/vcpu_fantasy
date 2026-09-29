# vCPU documentation

Just for anyone raging that I don't provide a Windows release I don't have a working Windows PC. (cross-compilation is bad too)

**_vCPU is a fantasy processor that features:_**
+ 64KB of ram
+ 3 registers (A - Math, X - Memory, Y - Results/Other)
+ 96x96 4-bit color screen

## Credits
This project uses:
* **tinyfiledialogs** - Included under the **zlib license** (see `tinyfiledialogs.h` for details).
* **font8x8** by Daniel Hepper - 8x8 bitmap font data, dedicated to the **Public Domain**.

## Opcodes
**The vCPU processor features _44_ opcodes**

#### Store value into register
+ STA - Loads value into A (ex. `STA 0x1234`)
+ STX - Loads value into X (ex. `STX 5678`)
+ STY - Loads value into Y (ex. `STY 1001100110011001b`)

#### Move registers
+ MAX - Move A into X
+ MAY - Move A into Y
+ MXA - Move X into A
+ MXY - Move X into Y
+ MYA - Move Y into A
+ MYX - Move Y into X

#### Memory operations
+ STM - Store value (pointer X) (value to write in Y)
+ LDM - Load value (pointer X) (read value in Y)

#### Math operations
+ ADD - Add A with Y
+ SUB - Subtract A with Y
+ MUL - Multiply A with Y
+ DIV - Divide A with Y (rest in X)

#### Bitwise/logic operations
+ SHR - Shift A right Y bits
+ SHL - Shift A left Y bits

+ AND - And A with Y
+ OR - Or A with Y
+ XOR - Xor A with Y
+ NOR - Nor A with Y

#### Subroutine opcodes
+ JMP - Jumps to subroutine
+ CALL - Pushses PC to stack and jumps to subroutine
+ RET - Jump to latest value in PC

#### Comparing operations
+ CMP - Compares A with Y
+ JZ - Jump if equal
+ JNZ - Jump if not equal
+ JL - Jump if less
+ JM - Jump if more

#### Clearing operations
+ CLF - Clears flags (zero, less, more)
+ CRD - Clears registers

#### Stack operations
+ PUSH - Pushes Y onto the stack
+ POP - Pops stack value into Y

#### Increment/decrement operations
+ INA - Increment A
+ INX - Increment X
+ INY - Increment Y

+ DEA - Decrement A
+ DEX - Decrement X
+ DEY - Decrement Y

#### Other operations
+ NOP - Does nothing
+ HLT - Exits program with code A
+ REPM - Repeats byte value in memory, value in A, X is the memory offset, Y is the amount
+ REPW - Repeats word value in memory, value in A, X is the memory offset, Y is the amount
+ SYSCALL - Makes a call to for ex. get a random value, A is the syscall num.

## Syscalls

+ 1 - Gets random value, returns in Y
+ 2 - Clears VRAM, color Y
+ 3 - Draws pixel, X and Y are the positions, value on stack is the color
+ 4 - Gets pressed key, returns in A
+ 5 - Checks if key is down, key in X, Y is the result
+ 6 - Prints character to screen, X and Y are the positions, value on stack is the character
+ 7 - Prints string, X and Y are the positions, value on stack is the string pointer

## Memory locations

The processor has 2 memory locations:
+ 0xF000-0xFFFF - VRAM (96x96, 4-bit color)
+ 0x1000-0xEFFF - Program (could still use the end if not large program)

## Definitions

To add subroutines/values use:

For subroutines:
```
jmp exSub

exSub:
ina
jmp exSub
```

For values (`.WORD` and `.BYTE`):
```
stx val1

val1:
.WORD 0xABCD, 0xDEAD, 0xBEEF
```

## Value types
There are _4_ types of values:
+ Hex - like 0x1234 or 1234h
+ Decimal - like 5678
+ Binary - like 0b0110011001100110 or 0110011001100110b
+ Character - like `%a` or `%A`

## Other infomation
To use comments use `;`.
Ex:
```
sta 0x1234
; Loads 0x1234 into register A
```
