#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <string>
#include <fstream>
#include <stack>

//define locations (ram, stack, registers)
uint16_t rom[0x10000];//Uses an aray to simulate ROM
uint16_t ram[0x10000]; //Uses an array to simulate RAM
uint16_t reg[0x10]; //16 registers
std::stack<uint16_t> stack; //stack
std::string fp; //filepath to ROM
bool term = false; //terminate, used to determine wether program has been ended
bool jump = false;

void loadrom() {
    std::ifstream file(fp);
    std::string line;
    size_t index = 0;

    while (std::getline(file, line) && index < 0x10000) {
        if (!line.empty() && line.back() == '%') {
            line.pop_back();
        }
        // Detect format
        if (line.rfind("0x", 0) == 0 || line.rfind("0X", 0) == 0) {
            // hex
            rom[index] = static_cast<uint16_t>(std::stoul(line, nullptr, 16));
        }
        else {
            // Binary
            rom[index] = static_cast<uint16_t>(std::stoul(line, nullptr, 2));
        }
        ++index;
    }
}

//INSTRUCTIONS
//Data Control
void MOV(uint16_t a, uint16_t b) {
    reg[b] = reg[a];
}
void MOVI(uint16_t a) {
    reg[0x9]++;
    reg[a] = rom[reg[0x9]];
}
void SWAP(uint16_t a, uint16_t b) {
    uint16_t temp = reg[a];
    reg[a] = reg[b];
    reg[b] = temp;
}
void LOAD(uint16_t a) {
    reg[0x9]++;
    uint16_t addr = rom[reg[0x9]];
    reg[a] = ram[addr];
}
void STORE(uint16_t a) {
    reg[0x9]++;
    uint16_t addr = rom[reg[0x9]];
    ram[addr] = reg[a];
}
void PUSH(uint16_t a){
    stack.push(reg[a]);
}
void POP(uint16_t a){
    if (stack.empty()){
        term = true;
        return;
    }

    reg[a] = stack.top();
    stack.pop();
}
//Arithmetic
void ADD(uint16_t a, uint16_t b)
{
    uint32_t full = (uint32_t)reg[a] + (uint32_t)reg[b];
    reg[0xA] = (uint16_t)full;

    reg[0xC] = (reg[0xA] == 0);
    reg[0xD] = (reg[0xA] & 0x8000);
    reg[0xE] = (full > 0xFFFF);
    reg[0xF] = ((~(reg[a] ^ reg[b]) & (reg[a] ^ reg[0xA]) & 0x8000) != 0);
}

void SUB(uint16_t a, uint16_t b)
{
    reg[0xA] = reg[a] - reg[b];

    reg[0xC] = (reg[0xA] == 0);
    reg[0xD] = (reg[0xA] & 0x8000);
    reg[0xE] = (reg[a] < reg[b]);
    reg[0xF] = (((reg[a] ^ reg[b]) & (reg[a] ^ reg[0xA]) & 0x8000) != 0);
}

void MUL(uint16_t a, uint16_t b)
{
    uint32_t full = (uint32_t)reg[a] * (uint32_t)reg[b];
    reg[0xA] = (uint16_t)full;

    reg[0xC] = (reg[0xA] == 0);
    reg[0xD] = (reg[0xA] & 0x8000);
    reg[0xE] = (full > 0xFFFF);
    reg[0xF] = 0;
}

void DIV(uint16_t a, uint16_t b)
{
    if (reg[b] == 0) { term = true; return; }

    reg[0xA] = reg[a] / reg[b];

    reg[0xC] = (reg[0xA] == 0);
    reg[0xD] = (reg[0xA] & 0x8000);
    reg[0xE] = 0;
    reg[0xF] = 0;
}

void INC(uint16_t a)
{
    ++reg[a];
    reg[0xA] = reg[a];

    reg[0xC] = (reg[0xA] == 0);
    reg[0xD] = (reg[0xA] & 0x8000);
    reg[0xE] = 0;
    reg[0xF] = (reg[0xA] == 0x8000);
}

void DEC(uint16_t a)
{
    --reg[a];
    reg[0xA] = reg[a];

    reg[0xC] = (reg[0xA] == 0);
    reg[0xD] = (reg[0xA] & 0x8000);
    reg[0xE] = 0;
    reg[0xF] = (reg[0xA] == 0x7FFF);
}
//Flow Control
void JMP() {reg[0x9]++; reg[0x9] = rom[reg[0x9]]; jump = true;}
void JMPZ() {if (reg[0xC]==0x1){JMP(); reg[0xC]=0x0;}}
void JMPS() {if (reg[0xD]==0x1){JMP(); reg[0xD]=0x0;}}
void JMPC() {if (reg[0xE]==0x1){JMP(); reg[0xE]=0x0;}}
void JMPO() {if (reg[0xF]==0x1){JMP(); reg[0xF]=0x0;}}
void CALL() {
    uint16_t ret_address = reg[0x9] + 2;
    stack.push(ret_address);
    JMP();
}
void RET() {
    if (stack.empty()) {term = true; return;}
    reg[0x9] = stack.top();
    stack.pop();
    jump = true;
}
//Bitwise Operations
void AND(uint16_t a, uint16_t b) { reg[0xA] = reg[a] & reg[b]; }
void OR(uint16_t a, uint16_t b)  { reg[0xA] = reg[a] | reg[b]; }
void XOR(uint16_t a, uint16_t b) { reg[0xA] = reg[a] ^ reg[b]; }
void NOT(uint16_t a)             { reg[0xA] = ~reg[a]; }
void LSHIFT(uint16_t a)          { reg[0xA] = reg[a] << 1; }
void RSHIFT(uint16_t a)          { reg[0xA] = reg[a] >> 1; }
//Extra Functions
void INCPC(){ // Incrememts the PC
    reg[0x9]++;
}

//mainloop
int main() {
    std::cout << "Enter path to rom: ";
    std:: cin >> fp;
    loadrom();
    while(term!=true){
        std::cout << "\nCurrent Cycle: " << reg[0x9];
        jump = false;
        //fetch
        reg[0xB] = rom[reg[0x9]]; //Take instruction from ROM and store it in the IR.
        //seperate individual components from instruction
        uint16_t opcode = (reg[0xB] >> 11) & 0x1F;
        uint16_t rega = (reg[0xB] >> 7)  & 0x0F;
        uint16_t regb = (reg[0xB] >> 3)  & 0x0F;
        uint16_t unass = reg[0xB] & 0x07;
        std::cout << "\nCurrent Instruction: " << opcode;
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
                PUSH(rega); break;
            case 7:
                POP(rega); break;
            case 8:
                ADD(rega, regb); break;
            case 9:
                SUB(rega, regb); break;
            case 10:
                DIV(rega, regb); break;
            case 11:
                MUL(rega, regb); break;
            case 12:
                INC(rega); break;
            case 13:
                DEC(rega); break;
            case 14:
                JMP(); break;
            case 15:
                JMPZ(); break;
            case 16:
                JMPC(); break;
            case 17:
                JMPS(); break;
            case 18:
                JMPO(); break;
            case 19:
                CALL(); break;
            case 20:
                RET(); break;
            case 21:
                AND(rega, regb); break;
            case 22:
                OR(rega, regb); break;
            case 23:
                NOT(rega); break;
            case 24:
                XOR(rega, regb); break;
            case 25:
                LSHIFT(rega); break;
            case 26:
                RSHIFT(rega); break;
            case 27:
                term = true; break;
            default:
                term = true; break;

        }
        //Increment PC
        std::cout << "\nAccumulator: " << reg[0xA] << "\n\n";
        if (jump == false) {INCPC();}
    }
    std::cout << "Program Terminated";
    return 0;
}
