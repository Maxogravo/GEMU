#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <fstream>

uint16_t rom[0x10000];
std::string filepath;
void loadrom(){
    std::ifstream romfile(filepath);
    
}