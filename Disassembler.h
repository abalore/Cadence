#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "defs.h"
#include <string>

class Disassembler
{
public:
    static void Init();
    static void AddUserLabel(word address, const std::string &name);
    static void ClearUserLabels();
    static void SetPoint(word address);
    static void GetNextInstruction(BYTE &instrLength, BYTE &opCode,
                                   std::string *label, std::string *address,
                                   std::string *bytes, std::string *instruction,
                                   int boundary = 0x10000);
    static int addr;
};

#endif // DISASSEMBLER_H
