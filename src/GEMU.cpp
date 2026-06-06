#include <cstdint>
#include <iostream>
#include <sys/types.h>

//define locations (ram, stack, registers)
uint16_t rom[0x10000];//Uses an aray to simulate ROM
uint16_t ram[0x10000]; //Uses an array to simulate RAM
uint16_t reg[0x10]; //16 registers
bool term = false; //terminate, used to determine wether program has been ended
bool jump = false;

//INSTRUCTIONS
//Data Control
void MOV(uint16_t a, uint16_t &b) {  b = a;  }
void MOVI(uint16_t &a) {reg[0x9]++; a = rom[reg[0x9]];}
void SWAP(uint16_t &a, uint16_t &b) {  uint16_t temp = a; a = b; b  = temp;  }
void LOAD(uint16_t &a) { reg[0x9]++; uint16_t addr = rom[reg[0x9]]; a = ram[addr];  }
void STORE(uint16_t a) { reg[0x9]++; uint16_t addr = rom[reg[0x9]]; ram[addr] = a;}
//Arithmetic
void ADD(uint16_t a, uint16_t b)  {reg[0xA] = a + b;}
void SUB(uint16_t a, uint16_t b) {reg[0xA] =  a - b;}
void DIV(uint16_t a, uint16_t b) {
    if (b == 0) {
        term = true;
        return;
    }
    reg[0xA] = a / b;
}
void MUL(uint16_t a, uint16_t b) {reg[0xA] = a*b;}
void INC(uint16_t &a) {a++;}
void DEC(uint16_t &a) {a--;}
//Flow Control
void JMP() {reg[0x9]++; reg[0x9] = rom[reg[0x9]]; jump = true;}
void JMPZ() {if (reg[0xC]==0x1){JMP();}}
void JMPS() {if (reg[0xD]==0x1){JMP();}}
void JMPC() {if (reg[0xE]==0x1){JMP();}}
void JMPO() {if (reg[0xF]==0x1){JMP();}}
//Bitwise Operations
void AND(uint16_t a, uint16_t b) {reg[0xA]=a&b;}
void OR(uint16_t a, uint16_t b) {reg[0xA]=a|b;}
void XOR(uint16_t a, uint16_t b) {reg[0xA]=a^b;}
void NOT(uint16_t a) {reg[0xA]=~a;}
void LSHIFT(uint16_t a) {reg[0xA] = a<<1;}
void RSHIFT(uint16_t a) {reg[0xA] = a>>1;}
//Program
void HALT() {term = true;}

//Extra Functions
void INCPC(){ // Incrememts the PC
    reg[0x9]++;
}

//mainloop
int main() {
    while(term!=true){
        jump = false;
        //fetch
        reg[0xB] = rom[reg[0x9]]; //Take instruction from ROM and store it in the IR.
        //seperate individual components from instruction
        uint16_t opcode = (reg[0xB] >> 11) & 0x1F;
        uint16_t rega = (reg[0xB] >> 7)  & 0x0F;
        uint16_t regb = (reg[0xB] >> 3)  & 0x0F;
        uint16_t unass = reg[0xB] & 0x07;
        //decode & execute
        switch (opcode) {
            case 0:
                break;
            case 1:
                MOV(rega,regb); break;
            case 2:
                MOVI(rega); break;
            case 3:
                SWAP(rega, regb); break;
            case 4:
                LOAD(rega); break;
            case 5:
                STORE(rega); break;
            case 6:
                ADD(rega, regb); break;
            case 7:
                SUB(rega, regb); break;
            case 8:
                DIV(rega, regb); break;
            case 9:
                MUL(rega, regb); break;
            case 10:
                INC(rega); break;
            case 11:
                DEC(rega); break;
            case 12:
                JMP(); break;
            case 13:
                JMPZ(); break;
            case 14:
                JMPC(); break;
            case 15:
                JMPS(); break;
            case 16:
                JMPO(); break;
            case 17:
                AND(rega, regb); break;
            case 18:
                OR(rega, regb); break;
            case 19:
                NOT(rega); break;
            case 20:
                XOR(rega, regb); break;
            case 21:
                LSHIFT(rega); break;
            case 22:
                RSHIFT(rega); break;
            case 23:
                HALT(); break;

        }
        //Increment PC
        if (jump == false) {INCPC();}
        std::cout << "\n" << reg[0x9];
    }
    std::cout << "Program Terminated";
    return 0;
}
