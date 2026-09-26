#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>
#include <stack>
#include "raylib.h"

//define locations (ram, stack, registers)
uint8_t rom[0x10000];//Uses an aray to simulate ROM
uint8_t ram[0x10000]; //Uses an array to simulate RAM
uint16_t reg[0x10]; //16 registers
uint16_t acc;
uint16_t ir;
uint16_t pc;
uint16_t flo;
uint16_t flc;
uint16_t flag_sign;
uint16_t flz;

int cycles;
int frames;
size_t romsize; //number of bytes actually loaded into ROM
std::stack<uint16_t> stack; //stack
std::string fp; //filepath to ROM
bool term = false; //terminate, used to determine wether program has been ended
bool jump = false;
char verbose; //Verbose output mode?
std::string inst; //Stores the current instruction to print
Color colors[22] = {
        BLACK, WHITE, ORANGE, DARKGRAY, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
        GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
        GREEN, SKYBLUE, PURPLE, BEIGE };

bool loadrom() {
    std::ifstream file(fp);
    if (!file) {
        std::cerr << "\nCould not open ROM: " << fp;
        return false;
    }

    std::string line;
    size_t index = 0;
    size_t lineno = 0;

    while (std::getline(file, line) && index < 0x10000) {
        ++lineno;
        size_t comment = line.find('%');
        if (comment != std::string::npos) {
            line.erase(comment);
        }
        size_t start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) {
            continue;
        }
        line = line.substr(start, line.find_last_not_of(" \t\r") - start + 1);

        try {
            // detect format
            if (line.rfind("0x", 0) == 0 || line.rfind("0X", 0) == 0) {
                // hex
                rom[index] = static_cast<uint8_t>(std::stoul(line, nullptr, 16));
            }
            else {
                // Binary
                rom[index] = static_cast<uint8_t>(std::stoul(line, nullptr, 2));
            }
        } catch (const std::exception &) {
            std::cerr << "\nInvalid byte on line " << lineno << " of " << fp << ": " << line;
            return false;
        }
        ++index;
    }

    romsize = index;
    if (romsize == 0) {
        std::cerr << "\nROM is empty: " << fp;
        return false;
    }
    return true;
}

void INCPC(){ // Incrememts the PC
    pc += 2;
}

//The CPU is 16bit but memory is 8bit, so two bytes are stitched
//together to build a single 16bit word.
uint16_t fetch_word(size_t addr) {
    if (addr + 1 >= romsize) {
        term = true;
        return 0;
    }
    return (static_cast<uint16_t>(rom[addr]) << 8) | rom[addr + 1];
}

uint16_t fetch_immediate() {
    uint16_t value = fetch_word(pc + 2);
    pc += 2;
    return value;
}

void ram_store_word(uint16_t addr, uint16_t value) {
    if (addr + 1 >= 0x10000) {
        term = true;
        return;
    }
    ram[addr] = static_cast<uint8_t>(value >> 8);
    ram[addr + 1] = static_cast<uint8_t>(value & 0xFF);
}

uint16_t ram_load_word(uint16_t addr) {
    if (addr + 1 >= 0x10000) {
        term = true;
        return 0;
    }
    return (static_cast<uint16_t>(ram[addr]) << 8) | ram[addr + 1];
}

void set_flags() {
    flz = (acc == 0);
    flag_sign = ((acc & 0x8000) != 0);
}

//INSTRUCTIONS
//Data Control
void MOV(uint16_t a, uint16_t b) {
    reg[b] = reg[a];
}
void MOVI(uint16_t a) {
    reg[a] = fetch_immediate();
}
void SWAP(uint16_t a, uint16_t b) {
    uint16_t temp = reg[a];
    reg[a] = reg[b];
    reg[b] = temp;
}
void LOAD(uint16_t a, uint16_t b) {
    uint16_t addr = (b == 0x0) ? fetch_immediate() : reg[b];
    reg[a] = ram_load_word(addr);
}
void STORE(uint16_t a, uint16_t b) {
    uint16_t addr = (b == 0x0) ? fetch_immediate() : reg[b];
    ram_store_word(addr, reg[a]);
}
void STOREI(){
    uint16_t val = fetch_immediate();
    uint16_t addr = fetch_immediate();
    ram_store_word(addr, val);
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
    uint16_t left = reg[a];
    uint16_t right = (b == 0x0) ? fetch_immediate() : reg[b];
    uint32_t full = static_cast<uint32_t>(left) + right;

    acc = static_cast<uint16_t>(full);
    reg[a] = acc;

    flc = (full > 0xFFFF);
    flo = ((~(left ^ right) & (left ^ acc) & 0x8000) != 0);
    set_flags();
}

void SUB(uint16_t a, uint16_t b)
{
    uint16_t left = reg[a];
    uint16_t right = (b == 0x0) ? fetch_immediate() : reg[b];
    uint16_t res = static_cast<uint16_t>(left - right);

    acc = res;
    reg[a] = res;

    flc = (left < right);
    flo = (((left ^ right) & (left ^ acc) & 0x8000) != 0);
    set_flags();
}

void MUL(uint16_t a, uint16_t b)
{
    uint16_t right = (b == 0x0) ? fetch_immediate() : reg[b];
    uint32_t full = static_cast<uint32_t>(reg[a]) * right;

    acc = static_cast<uint16_t>(full);
    reg[a] = acc;

    flc = (full > 0xFFFF);
    flo = 0;
    set_flags();
}

void DIV(uint16_t a, uint16_t b)
{
    uint16_t right = (b == 0x0) ? fetch_immediate() : reg[b];
    if (right == 0) { term = true; return; }

    uint16_t val = reg[a] / right;
    acc = val;
    reg[a] = val;

    flc = 0;
    flo = 0;
    set_flags();
}

void INC(uint16_t a)
{
    ++reg[a];
    acc = reg[a];

    flc = (acc == 0);
    flo = (acc == 0x8000);
    set_flags();
}

void DEC(uint16_t a)
{
    --reg[a];
    acc = reg[a];

    flc = (acc == 0xFFFF);
    flo = (acc == 0x7FFF);
    set_flags();
}
//Flow Control
void JMP() {pc = fetch_immediate(); jump = true;}
void JMPZ() {if (flz==0x1){JMP(); flz=0x0;}}
void JMPS() {if (flag_sign==0x1){JMP(); flag_sign=0x0;}}
void JMPC() {if (flc==0x1){JMP(); flc=0x0;}}
void JMPO() {if (flo==0x1){JMP(); flo=0x0;}}
void CALL() {
    uint16_t target = fetch_immediate();
    stack.push(static_cast<uint16_t>(pc + 2));
    pc = target;
    jump = true;
}
void RET() {
    if (stack.empty()) {term = true; return;}
    pc = stack.top();
    stack.pop();
    jump = true;
}
//Bitwise Operations
void AND(uint16_t a, uint16_t b) {
    acc = reg[a] = reg[a] & reg[b];
    flc = 0;
    flo = 0;
    set_flags();
}
void OR(uint16_t a, uint16_t b)  {
    acc = reg[a] = reg[a] | reg[b];
    flc = 0;
    flo = 0;
    set_flags();
}
void XOR(uint16_t a, uint16_t b) {
    acc = reg[a] = reg[a] ^ reg[b];
    flc = 0;
    flo = 0;
    set_flags();
}
void NOT(uint16_t a) {
    acc = reg[a] = ~reg[a];
    flc = 0;
    flo = 0;
    set_flags();
}
void LSHIFT(uint16_t a) {
    flc = ((reg[a] & 0x8000) != 0);
    acc = reg[a] = static_cast<uint16_t>(reg[a] << 1);
    flo = 0;
    set_flags();
}
void RSHIFT(uint16_t a) {
    flc = ((reg[a] & 0x1) != 0);
    acc = reg[a] = reg[a] >> 1;
    flo = 0;
    set_flags();
}
//Graphics
void CLS(){
    for (uint16_t locs = 0; locs < 4096; locs++) {ram[0xF000 + locs] = 0;}
}
void PXL(uint16_t a, uint16_t b) {
    uint16_t x = (a == 0x0) ? fetch_immediate() : reg[a];
    uint16_t y = (b == 0x0) ? fetch_immediate() : reg[b];
    uint16_t colour = fetch_immediate();

    if (x >= 64 || y >= 64) {return;}
    ram[0xF000 + y * 64 + x] = static_cast<uint8_t>(colour);
}

void draw_framebuffer() {
    int pixeladdr = 0xF000;
    BeginDrawing();
    ClearBackground(BLACK);
    for(int row = 0; row<64; row++){
        for(int col = 0; col<64; col++){
            DrawRectangle(col*10, row*10, 10,10, colors[ram[pixeladdr] % 22]);
            pixeladdr++;
        }
    }
    EndDrawing();
}

int main() {
    std::cout << "Enter path to rom: ";
    std:: cin >> fp;
    std::cout << "\nVerbose mode (y/n)? ";
    std::cin >> verbose;
    std::cout << "\nEnter FPS: ";
    std::cin >> frames;
    if (!loadrom()) { return 1; }

    InitWindow(640,640,"GEMU");
    SetTargetFPS(frames);

    while (!WindowShouldClose()) {
        while(!term){
            jump = false;
            if (pc + 1 >= romsize) {term = true; break;}
            //fetch
            ir = fetch_word(pc); //Take instruction from ROM and store it in the IR.
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
                    inst = "Invalid Opcode"; term = true; break;

            }
            draw_framebuffer();

            if (verbose == 'y' && term == false){
            std::cout << "\nCurrent Cycle: " << cycles;
            std::cout << "\nCurrent Instruction: " << inst;
            std::cout << "\nAccumulator Value: " << acc;
            }
            if (jump == false) {INCPC();}
            cycles++;
        }
        draw_framebuffer();
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
