# GEMU Docs

## Specs
Address Width: 16bits
Data Width: 16bits
ROM max size: 65535 words
RAM max size: 65535 words
Harvard Architecture. Program is stored in ROM and currently used data is stored in RAM.

## Instruction Set Architecture (ISA)
### Structure
[00000|0000|0000|000]
[opcode|reg a|reg b|unassigned]

### Instructions
No Operation
Move (reg a to reg b)
Move Immediate (immediate to reg a)
Swap (reg a, reg b)
Load (reg a = ram[immediate/reg b])
Store (ram[immediate/reg b] = reg a)
Store Immediate (ram[immediate] = immediate2)
Push (reg a)
Pop (reg a)
Add (reg a = reg a + immediate/reg b)
Sub (reg a = reg a - immediate/reg b)
Divide (reg a = reg a / immediate/reg b)
Multiply (reg a = reg a * immediate/reg b)
Increment (reg a = reg a++)
Decrement (reg a = reg b--)
Jump (immediate)
Jump Zero (immediate)
Jump Sign (immediate)
Jump Carry (immediate)
Jump Overflow (immediate)
Call (return address = immediate 1, jump address = immediate 2)
Return
And (reg a, reg b)
Or (reg a,reg b)
Xor (reg a, reg b)
Not (reg a)
Left Shift (reg a)
Right Shift (reg a)
Clear Video Buffer
Plot Pixel (x=reg a/immediate y=reg b/immediate colour=immediate)
Halt

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
fls (Sign flag (pos/neg))
flc (Carry flag)
flo (Overflow Flag)

## Memory Map (RAM)
0->61438 words = General Data
61439->65535 words =  Video Buffer (One word per pixel on 64x64 screen)

## Graphics
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

## GASM Language
GASM stands for Gravy Assembly. The name is based off the cpu (Gravy 16)
There are three main components of the language. The keyword, minor words and flags. 
The keyword will either be 'pass' (for no operation) or one which corresponds to an instruction in the ISA. e.g MOV
The minor word will be any registers that need defining. e.g ra OR any numbers. Numbers can either be in hexadecimal or 16bit binary. 
Numbers must have their own seperate line as the cpu reads it that way. This includes immediates and addresses.
Flags are just characters that tell the compiler to skip them or end line. " " and ";". 
Registers and the ISA are found in this folder.

### Example
MOVI ra; <- keyword: MOVI (Move immediate) minor word: ra. (move immediate to register ra). End of line is represented by semicolon 
0x6; <- Immediate value of 6 to be moved to register ra
DEC ra; <- Decrement register ra
JMPZ; <- Jump if zero (this one doesnt require any minor words)
0x7; <- Address in ROM to jump to
JMP; <- Sometimes an instruction doesnt need to specify any other registers. 
0x2;
HALT;