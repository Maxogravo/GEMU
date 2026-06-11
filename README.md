**GEMU**

GEMU is a mini emulator for a custom designed CPU - the Gravy 16, also complete with a mini language to produce programs for it!
I made it to learn and improve my c++ skills as I was quite new to the language while writing it. I also made it to learn and undertsand key concepts of low level computing. The reason I call this a mini emulator is because at this moment it only reads instructions and processes them. It does not currently have IO or graphics but I will be working to add these features. 

**How To Use**
1. Write a program in GASM (Gravy Assembly) as a .gasm file. Check docs/GASM.txt to learn it!
2. Compile by running GASM/compiler.py enter local path to .gasm and the local path which you'd like the compiled program to be stored. 
3. Compile GEMU (src/GEMU.cpp)
4. run GEMU and specify path to rom


**Current Features**
- Read binary/hexadecimal values from a .txt file and store in ROM
- FDE cycle
- Custom ISA
- Mini programming language based on assembly

