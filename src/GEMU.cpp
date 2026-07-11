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
uint16_t reg[0xF]; //16 registers
uint16_t acc;
uint16_t ir;
uint16_t pc;
uint16_t flo;
uint16_t flc;
uint16_t flag_sign;
uint16_t flz;

int cycles;
int frames;
std::stack<uint16_t> stack; //stack
std::string fp; //filepath to ROM
bool term = false; //terminate, used to determine wether program has been ended
bool jump = false;
char verbose; //Verbose output mode?
std::string inst; //Stores the current instruction to print
int pixeladdr;
Color colors[22] = {
        BLACK, WHITE, ORANGE, DARKGRAY, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
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
    pc++;
}

//INSTRUCTIONS
//Data Control
void MOV(uint16_t a, uint16_t b) {
    reg[b] = reg[a];
}
void MOVI(uint16_t a) {
    pc++;
    reg[a] = rom[pc];
}
void SWAP(uint16_t a, uint16_t b) {
    uint16_t temp = reg[a];
    reg[a] = reg[b];
    reg[b] = temp;
}
void LOAD(uint16_t a, uint16_t b) {
    uint16_t addr;
    if(b==0x0){pc++; addr = rom[pc];}else{addr = reg[b];}
    reg[a] = ram[addr];
}
void STORE(uint16_t a, uint16_t b) {
    uint16_t addr;
    if (b==0x0){
        pc++;
        addr = rom[pc];
    } else {addr = reg[b];}
    ram[addr] = reg[a];
}
void STOREI(){
    pc++;
    uint16_t val = rom[pc];
    pc++;
    ram[rom[pc]] = val;

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
    uint32_t full;
    if(b==0x0){
        pc++;
        full = (uint32_t)reg[a] + rom[pc];
    }else{full = (uint32_t)reg[a] + (uint32_t)reg[b];}
    acc = (uint16_t)full;
    reg[a] = (uint16_t)full;

    flz = (acc == 0);
    flag_sign = (acc & 0x8000);
    flc = (full > 0xFFFF);
    flo = ((~(reg[a] ^ reg[b]) & (reg[a] ^ acc) & 0x8000) != 0);
}

void SUB(uint16_t a, uint16_t b)
{
    uint32_t val;
    if (b==0){
        pc++;
        val = reg[a] - rom[pc];
    }else{val = reg[a]-reg[b];}
    acc = (uint16_t)val;
    reg[a] = (uint16_t)val;

    flz = (acc == 0);
    flag_sign = (acc & 0x8000);
    flc = (reg[a] < reg[b]);
    flo = (((reg[a] ^ reg[b]) & (reg[a] ^ acc) & 0x8000) != 0);
}

void MUL(uint16_t a, uint16_t b)
{
    uint32_t full;
    if (b==0x0){
        pc++;
        full = (uint32_t)reg[a] * rom[pc];
    }else{full = (uint32_t)reg[a] * (uint32_t)reg[b];}
    acc = (uint16_t)full;
    reg[a] = (uint16_t)full;

    flz = (acc == 0);
    flag_sign = (acc & 0x8000);
    flc = (full > 0xFFFF);
    flo = 0;
}

void DIV(uint16_t a, uint16_t b)
{
    uint16_t val;
    if (b==0){
        if(rom[pc++]!=0){
            pc++;
            val = reg[a] / rom[pc];
        }else{term=true;}
    }else{if (reg[b] == 0) { term = true; return; } val = reg[a]/reg[b];}

    acc = val;
    reg[a] = val;

    flz = (acc == 0);
    flag_sign = (acc & 0x8000);
    flc = 0;
    flo = 0;
}

void INC(uint16_t a)
{
    ++reg[a];
    acc = reg[a];

    flz = (acc == 0);
    flag_sign = (acc & 0x8000);
    flc = 0;
    flo = (acc == 0x8000);
}

void DEC(uint16_t a)
{
    --reg[a];
    acc = reg[a];

    flz = (acc == 0);
    flag_sign = (acc & 0x8000);
    flc = 0;
    flo = (acc == 0x7FFF);
}
//Flow Control
void JMP() {pc++; pc = rom[pc]; jump = true;}
void JMPZ() {if (flz==0x1){JMP(); flz=0x0;}}
void JMPS() {if (flag_sign==0x1){JMP(); flag_sign=0x0;}}
void JMPC() {if (flc==0x1){JMP(); flc=0x0;}}
void JMPO() {if (flo==0x1){JMP(); flo=0x0;}}
void CALL() {
    uint16_t ret_address = pc + 2;
    stack.push(ret_address);
    JMP();
}
void RET() {
    if (stack.empty()) {term = true; return;}
    pc = stack.top();
    stack.pop();
    jump = true;
}
//Bitwise Operations
void AND(uint16_t a, uint16_t b) { acc = reg[a] & reg[b]; }
void OR(uint16_t a, uint16_t b)  { acc = reg[a] | reg[b]; }
void XOR(uint16_t a, uint16_t b) { acc = reg[a] ^ reg[b]; }
void NOT(uint16_t a)             { acc = ~reg[a]; }
void LSHIFT(uint16_t a)          { acc = reg[a] << 1; }
void RSHIFT(uint16_t a)          { acc = reg[a] >> 1; }
//Graphics
void CLS(){
    for (uint16_t locs = 0; locs < 4096; locs++) {ram[0xF000 + locs] = 0;}
}
void PXL(uint16_t a, uint16_t b) {
    uint16_t x;
    uint16_t y;
    uint16_t colour;
    if (a==0){pc++; x = rom[pc];if(x>=64){return;}}else{x = reg[a];}
    if (b==0){pc++; y = rom[pc];if(y>=64){return;}}else{y = reg[b];}
    pc++;
    colour = rom[pc];
    ram[0xF000 + y * 64 + x] = colour;
}

int main() {
    std::cout << "Enter path to rom: ";
    std:: cin >> fp;
    std::cout << "\nVerbose mode (y/n)? ";
    std::cin >> verbose;
    std::cout << "\nEnter FPS: ";
    std::cin >> frames;
    loadrom();

    InitWindow(640,640,"GEMU");
    SetTargetFPS(frames);

    while (!WindowShouldClose()) {
        while(!term){
            jump = false;
            //fetch
            ir = rom[pc]; //Take instruction from ROM and store it in the IR.
            //seperate individual components from instruction
            uint16_t opcode = (ir >> 11) & 0x1F;
            uint16_t rega = (ir >> 7)  & 0x0F;
            uint16_t regb = (ir >> 3)  & 0x0F;
            uint16_t unass = ir & 0x07;
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
                    inst = "LOAD"; LOAD(rega, regb); break;
                case 5:
                    inst = "STORE"; STORE(rega, regb); break;
                case 6:
                    inst = "STORE IMMEDIATE"; STOREI(); break;
                case 7:
                    inst = "PUSH"; PUSH(rega); break;
                case 8:
                    inst = "POP"; POP(rega); break;
                case 9:
                    inst = "ADD"; ADD(rega, regb); break;
                case 10:
                    inst = "SUBTRACT"; SUB(rega, regb); break;
                case 11:
                    inst = "DIVIDE"; DIV(rega, regb); break;
                case 12:
                    inst = "MULTIPLY"; MUL(rega, regb); break;
                case 13:
                    inst = "INCREMENT"; INC(rega); break;
                case 14:
                    inst = "DECREMENT"; DEC(rega); break;
                case 15:
                    inst = "JUMP"; JMP(); break;
                case 16:
                    inst = "JUMP IF ZERO"; JMPZ(); break;
                case 17:
                    inst = "JUMP IF CARRY"; JMPC(); break;
                case 18:
                    inst = "JUMP IF NEGATIVE"; JMPS(); break;
                case 19:
                    inst = "JUMP IF OVERFLOW"; JMPO(); break;
                case 20:
                    inst = "CALL"; CALL(); break;
                case 21:
                    inst = "RETURN"; RET(); break;
                case 22:
                    inst = "AND"; AND(rega, regb); break;
                case 23:
                    inst = "OR"; OR(rega, regb); break;
                case 24:
                    inst = "NOT"; NOT(rega); break;
                case 25:
                    inst = "XOR"; XOR(rega, regb); break;
                case 26:
                    inst = "LEFT SHIFT"; LSHIFT(rega); break;
                case 27:
                    inst = "RIGHT SHIFT"; RSHIFT(rega); break;
                case 28:
                    inst = "Clear Screen"; CLS(); break;
                case 29:
                    inst = "Plot Pixel"; PXL(rega, regb); break;
                case 30:
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
            std::cout << "\nAccumulator Value: " << acc;
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