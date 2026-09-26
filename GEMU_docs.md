# GEMU Docs

## Specs
Address Width: 16bits
Data Width: 8bits (memory), 16bits (registers)
ROM max size: 65536 bytes
RAM max size: 65536 bytes
Harvard Architecture. Program is stored in ROM and currently used data is stored in RAM.

## Memory
ROM and RAM are byte addressable and hold 8 bit values. The CPU and its registers are
16 bit, so a 16 bit value occupies two consecutive memory locations, high byte first.
Whenever a register is loaded from or stored to memory, the low 8 bits live at the
address and the high 8 bits live at the address plus one.

## Instruction Set Architecture (ISA)
### Structure
[00000|0000|0000|000]
[opcode|reg a|reg b|unassigned]

An instruction is 16 bits wide but memory is only 8 bits wide, so the CPU reads two
consecutive bytes and stitches them together, high byte first, to build the single
instruction it fetches. Every instruction is therefore 2 bytes.

### Immediates
Immediates are 16 bit values that follow the instruction in ROM, stored as 2 bytes,
high byte first. So a MOVI (2 byte instruction plus 1 immediate) is 4 bytes, a
STORI (2 byte instruction plus 2 immediates) is 6 bytes, and a PXL using no registers
(2 byte instruction plus 3 immediates) is 8 bytes.

When an instruction names no register b, its second operand is read as an immediate
from ROM instead. When it does name register b, that register is used instead and no
immediate is consumed.

### Instructions
No Operation
Move (reg b = reg a)
Move Immediate (immediate to reg a)
Swap (reg a, reg b)
Load (reg a = ram[addr] and ram[addr+1] combined, addr = immediate/reg b)
Store (ram[addr] and ram[addr+1] = reg a split, addr = immediate/reg b)
Store Immediate (ram[immediate2] = immediate1)
Push (reg a)
Pop (reg a)
Add (reg a = reg a + immediate/reg b)
Sub (reg a = reg a - immediate/reg b)
Divide (reg a = reg a / immediate/reg b)
Multiply (reg a = reg a * immediate/reg b)
Increment (reg a = reg a++)
Decrement (reg a = reg a--)
Jump (immediate)
Jump Zero (immediate)
Jump Sign (immediate)
Jump Carry (immediate)
Jump Overflow (immediate)
Call (push the address of the following instruction, jump to immediate)
Return
And (reg a, reg b)
Or (reg a, reg b)
Xor (reg a, reg b)
Not (reg a)
Left Shift (reg a)
Right Shift (reg a)
Clear Video Buffer
Plot Pixel (x=reg a/immediate y=reg b/immediate colour=immediate)
Halt

Arithmetic, logic and shift instructions all write their result to both the accumulator
and register a. The bitwise instructions and the shifts also update the flags.

## Registers
### General Purpose 
These are registers that can be included or referenced in programs.
No Register
ra
rb
rc
rd
re
rf
rg
rh
ri
rj
rk
rl
rm
rn
ro

### Special Purpose
These serve other purposes
pc (Program Counter)
acc (Accumulator)
ir (Instruction Register)
flz (Zero flag)
flag_sign (Sign flag (pos/neg))
flc (Carry flag)
flo (Overflow Flag)

## Memory Map (RAM)
0->61439 bytes = General Data
61440->65535 bytes = Video Buffer (One byte per pixel on 64x64 screen)

## Graphics
### PXL command
It takes 3 inputs. Position x, position y and colour. Pixels outside the 64x64 screen
are discarded, but all 3 immediates are still consumed so the program counter stays
aligned to the next instruction.
### Colour Depth
BLACK, 
WHITE
ORANGE
DARKGRAY
DARKGREEN
DARKBLUE 
DARKPURPLE
DARKBROWN
GRAY
RED
GOLD
LIME
BLUE
VIOLET
BROWN
LIGHTGRAY
PINK 
YELLOW
GREEN
SKYBLUE
PURPLE 
BEIGE
Colours outside this palette wrap back to the start of the palette.

## GASM Language
GASM stands for Gravy Assembly. The name is based off the cpu (Gravy 16)
There are three main components of the language. The keyword, minor words and flags. 
The keyword will either be 'pass' (for no operation) or one which corresponds to an instruction in the ISA. e.g MOV
The minor word will be any registers that need defining. e.g ra OR any numbers. Numbers can either be in hexadecimal or 16bit binary. 
Numbers must have their own seperate line as the cpu reads it that way. This includes immediates and addresses.
Flags are just characters that tell the compiler to skip them or end line. " " and ";". 
Registers and the ISA are found in this folder.

### Output
The assembler writes a plain byte stream, one byte per line, where each byte is written
in hexadecimal. Instructions and immediates are both 16 bit values, so each one is split
into a high byte and a low byte across two lines. For example MOVI ra with the immediate
1 assembles to the four bytes 0x10 0x80 0x00 0x01, which is the instruction, then the
immediate, each as a high byte followed by a low byte.

### Example
MOVI ra; <- keyword: MOVI (Move immediate) minor word: ra. (move immediate to register ra). End of line is represented by semicolon 
0x6; <- Immediate value of 6 to be moved to register ra
DEC ra; <- Decrement register ra
JMPZ; <- Jump if zero (this one doesnt require any minor words)
0x7; <- Address in ROM to jump to
JMP; <- Sometimes an instruction doesnt need to specify any other registers. 
0x2;
HALT;
