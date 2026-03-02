#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "defs.h"
#include <string>
#include <map>

using namespace std;

class Disassembler
{
public:
    static void Init();
    static void SetPoint(word address);
    static void GetNextInstruction(BYTE &instrLength, BYTE &opCode, string *label, string *address, string *bytes, string *instruction);
    static int addr;
private:
    static BYTE *m;
    static string instr;
    static string bytes;
    static string address;
    static string idx;
    static string d;
    static word offset;
    static BYTE op;
    static BYTE length;
    static BYTE ReadNext();
    static string ReadHex8();
    static string ReadHex16();
    static string ReadSInt8();
    static string ReadRelativeAddressHex16();
    static void GetNextInstructionBasic();
    static void GetNextInstructionCB();
    static void GetNextInstructionIDXCB();
    static void GetNextInstructionIDX();
    static string GetRelativeIndex();
    static void GetNextInstructionIDX2();
    static void GetNextInstructionMisc();
    static void AddNewLabel(word address, string label);
    static const string CB_INSTR[];
    static const string CB_OPERAND[];
    static map<word, string> *labels;
};

#endif // DISASSEMBLER_H
