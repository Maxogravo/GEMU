import os
fp = input("Enter program filename: ")
fp = os.getcwd() + "/programs/" + fp
fn = input("Enter a name for the generated file: ")
fn = os.getcwd() + "/roms/" + fn + ".txt"
print(fn)

with open(fp) as f:
    for x in f:
        word = ""
        fields = []
        immediate = False
        opcode = "00000"
        rega = "0000"
        regb = "0000"
        extra = "000"
        stage = 0
        for y in x:
            if y == " " or y == ";":
                if word == "":
                    continue

                match word:
                    case "pass":
                        opcode = "00000"
                    case "MOV":
                        opcode = "00001"
                    case "MOVI":
                        opcode = "00010"
                    case "SWAP":
                        opcode = "00011"
                    case "LOAD":
                        opcode = "00100"
                    case "STOR":
                        opcode = "00101"
                    case "PUSH":
                        opcode = "00111"
                    case "POP":
                        opcode = "01000"
                    case "ADD":
                        opcode = "01001"
                    case "SUB":
                        opcode = "01010"
                    case "DIV":
                        opcode = "01011"
                    case "MUL":
                        opcode = "01100"
                    case "INC":
                        opcode = "01101"
                    case "DEC":
                        opcode = "01110"
                    case "JMP":
                        opcode = "01111"
                    case "JMPZ":
                        opcode = "10000"
                    case "JMPC":
                        opcode = "10001"
                    case "JMPS":
                        opcode = "10010"
                    case "JMPO":
                        opcode = "10011"
                    case "CALL":
                        opcode = "10100"
                    case "RET":
                        opcode = "10101"
                    case "AND":
                        opcode = "10110"
                    case "OR":
                        opcode = "10111"
                    case "NOT":
                        opcode = "11000"
                    case "XOR":
                        opcode = "11001"
                    case "LSHIFT":
                        opcode = "11010"
                    case "RSHIFT":
                        opcode = "11011"
                    case "CLS":
                        opcode = "11100"
                    case "PXL":
                        opcode = "11101"
                    case "HALT":
                        opcode = "11110"
                    case "STORI":
                        opcode = "00110"
                    case "nr":
                        opcode = '0000'
                    case "ra":
                        if stage == 1:
                            rega = "0001"
                        else:
                            regb = "0001"
                    case "rb":
                        if stage == 1:
                            rega = "0010"
                        else:
                            regb = "0010"
                    case "rc":
                        if stage == 1:
                            rega = "0011"
                        else:
                            regb = "0011"
                    case "rd":
                        if stage == 1:
                            rega = "0100"
                        else:
                            regb = "0100"
                    case "re":
                        if stage == 1:
                            rega = "0101"
                        else:
                            regb = "0101"
                    case "rf":
                        if stage == 1:
                            rega = "0110"
                        else:
                            regb = "0110"
                    case "rg":
                        if stage == 1:
                            rega = "0111"
                        else:
                            regb = "0111"
                    case "rh":
                        if stage == 1:
                            rega = "1000"
                        else:
                            regb = "1000"
                    case "ri":
                        if stage == 1:
                            rega = "1001"
                        else:
                            regb = "1001"
                    case "rj":
                        if stage == 1:
                            rega = "1010"
                        else:
                            regb = "1010"
                    case "rk":
                        if stage == 1:
                            rega = "1011"
                        else:
                            regb = "1011"
                    case "rl":
                        if stage == 1:
                            rega = "1100"
                        else:
                            regb = "1100"
                    case "rm":
                        if stage == 1:
                            rega = "1101"
                        else:
                            regb = "1101"
                    case "rn":
                        if stage == 1:
                            rega = "1110"
                        else:
                            regb = "1110"
                    case "ro":
                        if stage == 1:
                            rega = "1111"
                        else:
                            regb = "1111"

                    case _:
                        immediate = True
                        line = word

                stage += 1
                word = ""

            else:
                word += y

        if immediate == False:
            line = opcode + rega + regb + extra
        else:
            line = line
        
        with open(fn, "a") as nf:
            nf.write(line + "\n")
        print(line)