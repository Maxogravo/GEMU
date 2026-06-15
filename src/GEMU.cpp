#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <string>
#include <fstream>
#include <stack>
#include "raylib.h"

//define locations (ram, stack, registers)
uint16_t rom[0xFFFF];//Uses an aray to simulate ROM
uint16_t ram[0xFFFF]; //Uses an array to simulate RAM
uint16_t reg[0x10]; //16 registers
int cycles;
std::stack<uint16_t> stack; //stack
std::string fp; //filepath to ROM
bool term = false; //terminate, used to determine wether program has been ended
bool jump = false;
char verbose; //Verbose output mode?
std::string inst; //Stores the current instruction to print
int pixeladdr;
Color colors[21] = {
        DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
        GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
        GREEN, SKYBLUE, PURPLE, BEIGE };

void loadrom() {
    std::ifstream file(fp);
    std::string line;
    size_t index = 0;

    while (std::getline(file, line) && index < 0x10000) {
        if (!line.empty() && line.back() == '%') {
            line.pop_back();
        }
        // detect format
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

void INCPC(){ // Incrememts the PC
    reg[0x9]++;
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

int main() {
    std::cout << "Enter path to rom: ";
    std:: cin >> fp;
    std::cout << "\nVerbose mode (y/n)? ";
    std::cin >> verbose;
    loadrom();

    InitWindow(640,640,"GEMU");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        while(!term){
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
                    inst = "No Operation"; break;
                case 1:
                    inst = "MOV"; MOV(rega,regb); break;
                case 2:
                    inst = "MOV IMMEDIATE"; MOVI(rega); break;
                case 3:
                    inst = "SWAP"; SWAP(rega, regb); break;
                case 4:
                    inst = "LOAD"; LOAD(rega); break;
                case 5:
                    inst = "STORE"; STORE(rega); break;
                case 6:
                    inst = "PUSH"; PUSH(rega); break;
                case 7:
                    inst = "POP"; POP(rega); break;
                case 8:
                    inst = "ADD"; ADD(rega, regb); break;
                case 9:
                    inst = "SUBTRACT"; SUB(rega, regb); break;
                case 10:
                    inst = "DIVIDE"; DIV(rega, regb); break;
                case 11:
                    inst = "MULTIPLY"; MUL(rega, regb); break;
                case 12:
                    inst = "INCREMENT"; INC(rega); break;
                case 13:
                    inst = "DECREMENT"; DEC(rega); break;
                case 14:
                    inst = "JUMP"; JMP(); break;
                case 15:
                    inst = "JUMP IF ZERO"; JMPZ(); break;
                case 16:
                    inst = "JUMP IF CARRY"; JMPC(); break;
                case 17:
                    inst = "JUMP IF NEGATIVE"; JMPS(); break;
                case 18:
                    inst = "JUMP IF OVERFLOW"; JMPO(); break;
                case 19:
                    inst = "CALL"; CALL(); break;
                case 20:
                    inst = "RETURN"; RET(); break;
                case 21:
                    inst = "AND"; AND(rega, regb); break;
                case 22:
                    inst = "OR"; OR(rega, regb); break;
                case 23:
                    inst = "NOT"; NOT(rega); break;
                case 24:
                    inst = "XOR"; XOR(rega, regb); break;
                case 25:
                    inst = "LEFT SHIFT"; LSHIFT(rega); break;
                case 26:
                    inst = "RIGHT SHIFT"; RSHIFT(rega); break;
                case 27:
                    inst = "HALT"; term = true; break;
                default:
                    term = true; break;

            }
            pixeladdr = 0xF000;
            BeginDrawing();
            ClearBackground(BLACK);
            for(int row = 0; row<64; row++){
                for(int col = 0; col<64; col++){
                    DrawRectangle(col*10, row*10, 10,10, colors[ram[pixeladdr]]);
                    pixeladdr++;
                }
            }
            EndDrawing();

            if (verbose == 'y' and term == false){
            std::cout << "\nCurrent Cycle: " << cycles;
            std::cout << "\nCurrent Instruction: " << inst;
            std::cout << "\nAccumulator Value: " << reg[0xA];
            }
            if (jump == false) {INCPC();}
            cycles++;
        }
        pixeladdr = 0xF000;
        BeginDrawing();
        ClearBackground(BLACK);
        for(int row = 0; row<64; row++){
            for(int col = 0; col<64; col++){
                DrawRectangle(col*10, row*10, 10,10, colors[ram[pixeladdr]]);
                pixeladdr++;
            }
        }
        EndDrawing();
    }
    //Program Terminated
    std::cout << "\n\nProgram Terminated";
    std::cout << "\nRegister ra: " << reg[0x1];
    std::cout << "\nRegister rb: " << reg[0x2];
    std::cout << "\nRegister rc: " << reg[0x3];
    std::cout << "\nRegister rd: " << reg[0x4];
    std::cout << "\nRegister re: " << reg[0x5];
    std::cout << "\nRegister rf: " << reg[0x6];
    std::cout << "\nRegister rg: " << reg[0x7];
    std::cout << "\nRegister rh: " << reg[0x8] << "\n";
    CloseWindow();
}