#include "Headers/Disassembler.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"
#include "Headers/GateArray.h"
#include <stdlib.h>

using namespace std;

const string Disassembler::CB_INSTR[] =
    {
        "RLC ", "RRC ", "RL ", "RR ", "SLA ", "SRA ", "SLL ", "SRL ",
        "BIT 0,", "BIT 1,", "BIT 2,", "BIT 3,", "BIT 4,", "BIT 5,", "BIT 6,", "BIT 7,",
        "RES 0,", "RES 1,", "RES 2,", "RES 3,", "RES 4,", "RES 5,", "RES 6,", "RES 7,",
        "SET 0,", "SET 1,", "SET 2,", "SET 3,", "SET 4,", "SET 5,", "SET 6,", "SET 7,"
};

const string Disassembler::CB_OPERAND[] =
    {
        "B", "C", "D", "E", "H", "L", "(HL)", "A"
};

int Disassembler::addr;
BYTE *Disassembler::m;
word Disassembler::offset = 0;
string Disassembler::instr = "";
string Disassembler::bytes = "";
string Disassembler::address = "";
string Disassembler::idx = "";
string Disassembler::d;
BYTE Disassembler::op;
BYTE Disassembler::length;
map<word, string> *Disassembler::labels = new map<word, string>();

char buff[100];

void Disassembler::Init()
{
    /*
    // Oh mummy routines
    AddNewLabel(0x66ED, "GameLoop");
    AddNewLabel(0x7578, "UpdateMummies");       // returns fC = Game Over
    AddNewLabel(0x77D1, "UpdateMan");
    AddNewLabel(0x7863, "PrintScore");
    AddNewLabel(0x7893, "CheckForPause");
    AddNewLabel(0x78B7, "DrawMummies");
    AddNewLabel(0x78F7, "DrawMan");
    AddNewLabel(0x790B, "MoveMan");
    AddNewLabel(0x7A64, "ChkWallCollide");      // returns fZ = Collision
    AddNewLabel(0x7A98, "DrawMan");
    AddNewLabel(0x7AB0, "MoveUp");
    AddNewLabel(0x7AAC, "MoveRight");
    AddNewLabel(0x7AA6, "MoveDown");
    AddNewLabel(0x7AA2, "MoveLeft");
    AddNewLabel(0x7B39, "DrawSprType");         // A = Type (0x54 footsteps, 0x41 man), D = pos X, E = pos Y
    AddNewLabel(0x7D3E, "GetMapCoords");        // D = Line,  L = H byte,  returns HL
    AddNewLabel(0x7CC4, "DrawSprite");          // D = Line,  L = H byte,  IY = PixelData
    AddNewLabel(0x7CE6, "DrwFootOrEmpty");
    AddNewLabel(0x7E92, "GetSprLineAddr");      // H = Line,  L = H byte

    // Oh Mummy data
    AddNewLabel(0x7CC6, "SPRSIZEY");            // Self modified code
    AddNewLabel(0x7CD0, "SPRSIZEX");            // Self modified code
    AddNewLabel(0x814C, "KEY_ASSIGNS");
    AddNewLabel(0x8153, "DELAY");
    AddNewLabel(0x8155, "MAN_X");
    AddNewLabel(0x8155, "MAN_Y");
    AddNewLabel(0x8155, "LAST_DIRECTION");
    AddNewLabel(0x8158, "ANIM_STATE");
    AddNewLabel(0x815A, "SCORE");
    AddNewLabel(0x816A, "MEN");
    AddNewLabel(0x816C, "NUM_MUMMIES");
    AddNewLabel(0x816D, "MUMMIES_TABLE");
    AddNewLabel(0x8200, "MAP");                 // In chars, 40 x 25
*/

    // Boulder dash routines
    AddNewLabel(0x0206, "DrawTileInner2");
    AddNewLabel(0x028A, "DrawChar");
    AddNewLabel(0x030A, "DrawCharInner");
    AddNewLabel(0x035F, "DrawTileInner");
    AddNewLabel(0x039D, "DrawTile");
    AddNewLabel(0x03FD, "GetRandomNumber");

    AddNewLabel(0x10E9, "DrawTitle");
    AddNewLabel(0x1E92, "Init");
    AddNewLabel(0x1F42, "Start");
    AddNewLabel(0x1E2F, "MenuOrGame");
    AddNewLabel(0x1A2F, "RunMenu");
    AddNewLabel(0x1B0A, "DrawLevelEffect");
    AddNewLabel(0x20E8, "DrawFSLogo");
    AddNewLabel(0x2882, "SetPalette");

    // Boulder Dash data
    AddNewLabel(0x0064, "TileX");
    AddNewLabel(0x0065, "TileY");
    AddNewLabel(0x30E7, "BlackPalette");
    AddNewLabel(0x0133, "ScreenBase");
    AddNewLabel(0xB0E0, "Map");

    // BIOS routines
    AddNewLabel(0xBB09, "KMReadChar");
    AddNewLabel(0xBB0C, "KMCharReturn");
    AddNewLabel(0xBB1E, "KMTestKey");
    AddNewLabel(0xBB5A, "TXTOutput");
    AddNewLabel(0xBB66, "TXTWinEnable");
    AddNewLabel(0xBB75, "TXTSetCursor");
    AddNewLabel(0xBB90, "TXTSetPen");
    AddNewLabel(0xBB96, "TXTSetPaper");
    AddNewLabel(0x8ECA, "LineAddrTable");
    AddNewLabel(0xBB09, "KMReadChar");
    AddNewLabel(0xBCAA, "SoundQueue");
}

BYTE Disassembler::ReadNext()
{
    BYTE b = GetActiveMemory();
    sprintf(buff, "%02hhX ", static_cast<unsigned char>(b));
    bytes += buff;
    length++;
    addr++;
    return b;
}

string Disassembler::ReadRelativeAddressHex16()
{
    sbyte s = (sbyte)ReadNext();
    int jaddr = addr + s;
    sprintf(buff, "&%04X", jaddr);
    return (string)buff;
}

string Disassembler::ReadHex8()
{
    BYTE s = ReadNext();
    sprintf(buff, "&%02hhX", static_cast<unsigned char>(s));
    return (string)buff;
}

string Disassembler::ReadHex16()
{
    word w = (int)ReadNext() + (int)ReadNext() * 256;
    auto pos = labels->find(w);
    if (pos != labels->end())
        return pos->second;
    sprintf(buff, "&%04X", w);
    return (string)buff;
}

string Disassembler::ReadSInt8()
{
    sbyte s = (sbyte)ReadNext();
    sprintf(buff, "%+d", s);
    return (string)buff;
}

void Disassembler::SetPoint(ushort address)
{
    addr = address;
}

void Disassembler::AddNewLabel(word address, string label)
{
    labels->insert_or_assign(address, label);
}

BYTE Disassembler::GetActiveMemory()
{
    int bank = addr >> 14;
    int address = addr & 0x3FFF;
    switch(bank)
    {
    case 0:
        if (!GateArray::LoROMActive)
            return CPC::LoROM[address];
        else
            return CPC::RAM[bank][address];
        break;
    case 1:
    case 2:
        return CPC::RAM[bank][address];
        break;
    case 3:
        if (!GateArray::HiROMActive)
            return CPC::HiROM[address];
        else
            return CPC::RAM[bank][address];
    }
    return 0;
}

void Disassembler::GetNextInstruction(BYTE &instrLength, BYTE &opCode, string *label, string *addressStr, string *bytesStr, string *instrStr)
{
    auto pos = labels->find(addr);
    if (pos != labels->end())
        *label = pos->second;
    else
        *label = (string)"";
    length = 0;
    sprintf(buff, "%04X", addr);
    *addressStr = (string)buff;
    instr = "??";
    bytes = "";
    op = ReadNext();
    opCode = op;
    GetNextInstructionBasic();
    *instrStr = instr;
    *bytesStr = bytes;
    instrLength = length;
}

void Disassembler::GetNextInstructionBasic()
{
    switch(op >> 4)
    {
    case 0x0:
        switch(op & 0x0F)
        {
        case 0x0: // NOP
            instr = "NOP";
            break;
        case 0x1: // LD BC,nn
            instr = "LD BC," + ReadHex16();
            break;
        case 0x2: // LD (BC),A
            instr = "LD (BC),A";
            break;
        case 0x3: // INC BC
            instr = "INC BC";
            break;
        case 0x4: // INC B
            instr = "INC B";
            break;
        case 0x5: // DEC B
            instr = "DEC B";
            break;
        case 0x6: // LD B,n
            instr = "LD B," + ReadHex8();
            break;
        case 0x7: // RLCA
            instr = "RLCA";
            break;
        case 0x8: // EX AF,AF'
            instr = "EX AF,AF'";
            break;
        case 0x9: // ADD HL,BC
            instr = "ADD HL,BC";
            break;
        case 0xA: // LD A,(BC)
            instr = "LD A,(BC)";
            break;
        case 0xB: // DEC BC;
            instr = "DEC BC";
            break;
        case 0xC: // INC C
            instr = "INC C";
            break;
        case 0xD: // DEC C
            instr = "DEC C";
            break;
        case 0xE: // LD C,n
            instr = "LD C," + ReadHex8();
            break;
        case 0xF: // RRCA
            instr = "RRCA";
            break;
        }
        break;
    case 0x1:
        switch(op & 0x0F)
        {
        case 0x0: // DJNZ d
            instr = "DJNZ " + ReadRelativeAddressHex16();
            break;
        case 0x1: // LD DE,nn
            instr = "LD DE," + ReadHex16();
            break;
        case 0x2: // LD (DE),A
            instr = "LD (DE),A";
            break;
        case 0x3: // INC DE
            instr = "INC DE";
            break;
        case 0x4: // INC D
            instr = "INC D";
            break;
        case 0x5: // DEC D
            instr = "DEC D";
            break;
        case 0x6: // LD D,n
            instr = "LD D," + ReadHex8();
            break;
        case 0x7: // RLA
            instr = "RLA";
            break;
        case 0x8: // JR d
            instr = "JR " + ReadRelativeAddressHex16();
            break;
        case 0x9: // ADD HL,DE
            instr = "ADD HL,DE";
            break;
        case 0xA: // LD A,(DE)
            instr = "LD A,(DE)";
            break;
        case 0xB: // DEC DE
            instr = "DEC DE";
            break;
        case 0xC: // INC E
            instr = "INC E";
            break;
        case 0xD: // DEC E
            instr = "DEC E";
            break;
        case 0xE: // LD E,n
            instr = "LD E," + ReadHex8();
            break;
        case 0xF: // RRA
            instr = "RRA";
            break;
        }
        break;
    case 0x2:
        switch(op & 0x0F)
        {
        case 0x0: // JR NZ,d
            instr = "JR NZ," + ReadRelativeAddressHex16();
            break;
        case 0x1: // LD HL,nn
            instr = "LD HL," + ReadHex16();
            break;
        case 0x2: //  LD (nn),HL
            instr = "LD (" + ReadHex16() + "),HL";
            break;
        case 0x3: // INC HL
            instr = "INC HL";
            break;
        case 0x4: // INC H
            instr = "INC H";
            break;
        case 0x5: // DEC H
            instr = "DEC H";
            break;
        case 0x6: // LD H,n
            instr = "LD H," + ReadHex8();
            break;
        case 0x7: // DAA
            instr = "DAA";
            break;
        case 0x8: // JR Z,d
            instr = "JR Z," + ReadRelativeAddressHex16();
            break;
        case 0x9: // ADD HL,HL
            instr = "ADD HL,HL";
            break;
        case 0xA: // LD HL,(nn)
            instr = "LD HL,(" + ReadHex16() + ")";
            break;
        case 0xB: // DEC HL
            instr = "DEC HL";
            break;
        case 0xC: // INC L
            instr = "INC L";
            break;
        case 0xD: // DEC L
            instr = "DEC L";
            break;
        case 0xE: // LD L,n
            instr = "LD L," + ReadHex8();
            break;
        case 0xF: // CPL
            instr = "CPL";
            break;
        }
        break;
    case 0x3:
        switch(op & 0x0F)
        {
        case 0x0: // JR NC,D
            instr = "JR NC," + ReadRelativeAddressHex16();
            break;
        case 0x1: // LD SP,nn
            instr = "LD SP," + ReadHex16();
            break;
        case 0x2: // LD (nn),A
            instr = "LD (" + ReadHex16() + "),A";
            break;
        case 0x3: // INC SP
            instr = "INC SP";
            break;
        case 0x4: // INC (HL)
            instr = "INC (HL)";
            break;
        case 0x5: // DEC (HL)
            instr = "DEC (HL)";
            break;
        case 0x6: // LD (HL),n
            instr = "LD (HL)," + ReadHex8();
            break;
        case 0x7: // SCF
            instr = "SCF";
            break;
        case 0x8: // JR C,d
            instr = "JR C," + ReadRelativeAddressHex16();
            break;
        case 0x9: // ADD HL,SP
            instr = "ADD HL,SP";
            break;
        case 0xA: // LD A,(nn)
            instr = "LD A,(" + ReadHex16() + ")";
            break;
        case 0xB: // DEC SP
            instr = "DEC SP";
            break;
        case 0xC: // INC A
            instr = "INC A";
            break;
        case 0xD: // DEC A
            instr = "DEC A";
            break;
        case 0xE: // LD A,n
            instr = "LD A," + ReadHex8();
            break;
        case 0xF: // CCF
            instr = "CCF";
            break;
        }
        break;
    case 0x4:
        switch(op & 0x0F)
        {
        case 0x0: // LD B,B
            instr = "LD B,B";
            break;
        case 0x1: // LD B,C
            instr = "LD B,C";
            break;
        case 0x2: // LD B,D
            instr = "LD B,D";
            break;
        case 0x3: // LD B,E
            instr = "LD B,E";
            break;
        case 0x4: // LD B,H
            instr = "LD B,H";
            break;
        case 0x5: // LD B,L
            instr = "LD B,L";
            break;
        case 0x6: // LD B,(HL)
            instr = "LD B,(HL)";
            break;
        case 0x7: // LD B,A
            instr = "LD B,A";
            break;
        case 0x8: // LD C,B
            instr = "LD C,B";
            break;
        case 0x9: // LD C,C
            instr = "LD C,C";
            break;
        case 0xA: // LD C,D
            instr = "LD C,D";
            break;
        case 0xB: // LD C,E
            instr = "LD C,E";
            break;
        case 0xC: // LD C,H
            instr = "LD C,H";
            break;
        case 0xD: // LD C,L
            instr = "LD C,L";
            break;
        case 0xE: // LD C,(HL)
            instr = "LD C,(HL)";
            break;
        case 0xF: // LD C,A
            instr = "LD C,A";
            break;
        }
        break;
    case 0x5:
        switch(op & 0x0F)
        {
        case 0x0: // LD D,B
            instr = "LD D,B";
            break;
        case 0x1: // LD D,C
            instr = "LD D,C";
            break;
        case 0x2: // LD D,D
            instr = "LD D,D";
            break;
        case 0x3: // LD D,E
            instr = "LD D,E";
            break;
        case 0x4: // LD D,H
            instr = "LD D,H";
            break;
        case 0x5: // LD D,L
            instr = "LD D,L";
            break;
        case 0x6: // LD D,(HL)
            instr = "LD D,(HL)";
            break;
        case 0x7: // LD D,A
            instr = "LD D,A";
            break;
        case 0x8: // LD E,B
            instr = "LD E,B";
            break;
        case 0x9: // LD E,C
            instr = "LD E,C";
            break;
        case 0xA: // LD E,D
            instr = "LD E,D";
            break;
        case 0xB: // LD E,E
            instr = "LD E,E";
            break;
        case 0xC: // LD E,H
            instr = "LD E,H";
            break;
        case 0xD: // LD E,L
            instr = "LD E,L";
            break;
        case 0xE: // LD E,(HL)
            instr = "LD E,(HL)";
            break;
        case 0xF: // LD E,A
            instr = "LD E,A";
            break;
        }
        break;
    case 0x6:
        switch(op & 0x0F)
        {
        case 0x0: // LD H,B
            instr = "LD H,B";
            break;
        case 0x1: // LD H,C
            instr = "LD H,C";
            break;
        case 0x2: // LD H,D
            instr = "LD H,D";
            break;
        case 0x3: // LD H,E
            instr = "LD H,E";
            break;
        case 0x4: // LD H,H
            instr = "LD H,H";
            break;
        case 0x5: // LD H,L
            instr = "LD H,L";
            break;
        case 0x6: // LD H,(HL)
            instr = "LD H,(HL)";
            break;
        case 0x7: // LD H,A
            instr = "LD H,A";
            break;
        case 0x8: // LD L,B
            instr = "LD L,B";
            break;
        case 0x9: // LD L,C
            instr = "LD L,C";
            break;
        case 0xA: // LD L,D
            instr = "LD L.D";
            break;
        case 0xB: // LD L,E
            instr = "LD L,E";
            break;
        case 0xC: // LD L,H
            instr = "LD L,H";
            break;
        case 0xD: // LD L,L
            instr = "LD L,L";
            break;
        case 0xE: // LD L,(HL)
            instr = "LD L,(HL)";
            break;
        case 0xF: // LD L,A
            instr = "LD L,A";
            break;
        }
        break;
    case 0x7:
        switch(op & 0x0F)
        {
        case 0x0: // LD (HL),B
            instr = "LD (HL),B";
            break;
        case 0x1: // LD (HL),C
            instr = "LD (HL),C";
            break;
        case 0x2: // LD (HL),D
            instr = "LD (HL),D";
            break;
        case 0x3: // LD (HL),E
            instr = "LD (HL),E";
            break;
        case 0x4: // LD (HL),H
            instr = "LD (HL),H";
            break;
        case 0x5: // LD (HL),L
            instr = "LD (HL),L";
            break;
        case 0x6: // HALT
            instr = "HALT";
            break;
        case 0x7: // LD (HL),A
            instr = "LD (HL),A";
            break;
        case 0x8: // LD A,B
            instr = "LD A,B";
            break;
        case 0x9: // LD A,C
            instr = "LD A,C";
            break;
        case 0xA: // LD A,D
            instr = "LD A,D";
            break;
        case 0xB: // LD A,E
            instr = "LD A,E";
            break;
        case 0xC: // LD A,H
            instr = "LD A,H";
            break;
        case 0xD: // LD A,L
            instr = "LD A,L";
            break;
        case 0xE: // LD A,(HL)
            instr = "LD A,(HL)";
            break;
        case 0xF: // LD A,A
            instr = "LD A,A";
            break;
        }
        break;
    case 0x8:
        switch(op & 0x0F)
        {
        case 0x0: // ADD B
            instr = "ADD B";
            break;
        case 0x1: // ADD C
            instr = "ADD C";
            break;
        case 0x2: // ADD D
            instr = "ADD D";
            break;
        case 0x3: // ADD E
            instr = "ADD E";
            break;
        case 0x4: // ADD H
            instr = "ADD H";
            break;
        case 0x5: // ADD L
            instr = "ADD K";
            break;
        case 0x6: // ADD (HL)
            instr = "ADD (HL)";
            break;
        case 0x7: // ADD A
            instr = "ADD A";
            break;
        case 0x8: // ADC B
            instr = "ADC B";
            break;
        case 0x9: // ADC C
            instr = "ADC C";
            break;
        case 0xA: // ADC D
            instr = "ADC D";
            break;
        case 0xB: // ADC E
            instr = "ADC E";
            break;
        case 0xC: // ADC H
            instr = "ADC H";
            break;
        case 0xD: // ADC L
            instr = "ADC L";
            break;
        case 0xE: // ADC (HL)
            instr = "ADC (HL)";
            break;
        case 0xF: // ADC A
            instr = "ADC A";
            break;
        }
        break;
    case 0x9:
        switch(op & 0x0F)
        {
        case 0x0: // SUB B
            instr = "SUB B";
            break;
        case 0x1: // SUB C
            instr = "SUB C";
            break;
        case 0x2: // SUB D
            instr = "SUB D";
            break;
        case 0x3: // SUB E
            instr = "SUB E";
            break;
        case 0x4: // SUB H
            instr = "SUB H";
            break;
        case 0x5: // SUB L
            instr = "SUB L";
            break;
        case 0x6: // SUB (HL)
            instr = "SUB (HL)";
            break;
        case 0x7: // SUB A
            instr = "SUB A";
            break;
        case 0x8: // SBC B
            instr = "SBC B";
            break;
        case 0x9: // SBC C
            instr = "SBC C";
            break;
        case 0xA: // SBC D
            instr = "SBC D";
            break;
        case 0xB: // SBC E
            instr = "SBC E";
            break;
        case 0xC: // SBC H
            instr = "SBC H";
            break;
        case 0xD: // SBC L
            instr = "SBC L";
            break;
        case 0xE: // SBC (HL)
            instr = "SBC (HL)";
            break;
        case 0xF: // SBC A
            instr = "SBC A";
            break;
        }
        break;
    case 0xA:
        switch(op & 0x0F)
        {
        case 0x0: // AND B
            instr = "AND B";
            break;
        case 0x1: // AND C
            instr = "AND C";
            break;
        case 0x2: // AND D
            instr = "AND D";
            break;
        case 0x3: // AND e
            instr = "AND E";
            break;
        case 0x4: // AND H
            instr = "AND H";
            break;
        case 0x5: // AND L
            instr = "AND L";
            break;
        case 0x6: // AND (HL)
            instr = "AND (HL)";
            break;
        case 0x7: // AND A
            instr = "AND A";
            break;
        case 0x8: // XOR B
            instr = "XOR B";
            break;
        case 0x9: // XOR c
            instr = "XOR C";
            break;
        case 0xA: // XOR D
            instr = "XOR D";
            break;
        case 0xB: // XOR E
            instr = "XOR E";
            break;
        case 0xC: // XOR H
            instr = "XOR H";
            break;
        case 0xD: // XOR L
            instr = "XOR L";
            break;
        case 0xE: // XOR (HL)
            instr = "XOR (HL)";
            break;
        case 0xF: // XOR A
            instr = "XOR A";
            break;
        }
        break;
    case 0xB:
        switch(op & 0x0F)
        {
        case 0x0: // OR B
            instr = "OR B";
            break;
        case 0x1: // OR C
            instr = "OR C";
            break;
        case 0x2: // OR D
            instr = "OR D";
            break;
        case 0x3: // OR E
            instr = "OR E";
            break;
        case 0x4: // OR H
            instr = "OR H";
            break;
        case 0x5: // OR L
            instr = "OR L";
            break;
        case 0x6: // OR (HL)
            instr = "OR (HL)";
            break;
        case 0x7: // OR A
            instr = "OR A";
            break;
        case 0x8: // CP B
            instr = "CP B";
            break;
        case 0x9: // CP C
            instr = "CP C";
            break;
        case 0xA: // CP D
            instr = "CP D";
            break;
        case 0xB: // CP E
            instr = "CP E";
            break;
        case 0xC: // CP H
            instr = "CP H";
            break;
        case 0xD: // CP L
            instr = "CP L";
            break;
        case 0xE: // CP (HL)
            instr = "CP (HL)";
            break;
        case 0xF: // CP A
            instr = "CP A";
            break;
        }
        break;
    case 0xC:
        switch(op & 0x0F)
        {
        case 0x0: // RET NZ
            instr = "RET NZ";
            break;
        case 0x1: // POP BC
            instr = "POP BC";
            break;
        case 0x2: // JP NZ,nn
            instr = "JP NZ," + ReadHex16();
            break;
        case 0x3: // JP nn
            instr = "JP " + ReadHex16();
            break;
        case 0x4: // CALL NZ,nn
            instr = "CALL NZ," + ReadHex16();
            break;
        case 0x5: // PUSH BC
            instr = "PUSH BC";
            break;
        case 0x6: // ADD A,n
            instr = "ADD A," + ReadHex8();
            break;
        case 0x7: // RST 00h
            instr = "RST 00";
            break;
        case 0x8: // RET Z
            instr = "RET Z";
            break;
        case 0x9: // RET
            instr = "RET";
            break;
        case 0xA: // JP Z,nn
            instr = "JP Z," + ReadHex16();
            break;
        case 0xB: // BIT OP
            GetNextInstructionCB();
            break;
        case 0xC: // CALL Z,nn
            instr = "CALL Z," + ReadHex16();
            break;
        case 0xD: // CALL nn
            instr = "CALL " + ReadHex16();
            break;
        case 0xE: // ADC A,n
            instr = "ADC A," + ReadHex8();
            break;
        case 0xF: // RST 08h
            instr = "RST 08";
            break;
        }
        break;
    case 0xD:
        switch(op & 0x0F)
        {
        case 0x0: // RET NC
            instr = "RET NC";
            break;
        case 0x1: // POP DE
            instr = "POP DE";
            break;
        case 0x2: // JP NC,NN
            instr = "JP NC," + ReadHex16();
            break;
        case 0x3: // OUT (n),A
            instr = "OUT " + ReadHex8() + ",A";
            break;
        case 0x4: // CALL NC,nn
            instr = "CALL NC," + ReadHex16();
            break;
        case 0x5: // PUSH_DE
            instr = "PUSH DE";
            break;
        case 0x6: // SUB n
            instr = "SUB " + ReadHex8();
            break;
        case 0x7: // RST 10h
            instr = "RST 10";
            break;
        case 0x8: // RET C
            instr = "RET C";
            break;
        case 0x9: // EXX
            instr = "EXX";
            break;
        case 0xA: // JP C,nn
            instr = "JP C," + ReadHex16();
            break;
        case 0xB: // IN A,(n)
            instr = "IN A," + ReadHex8();
            break;
        case 0xC: // CALL C,nn
            instr = "CALL C," + ReadHex16();
            break;
        case 0xD: // IX OP
            idx = "IX";
            GetNextInstructionIDX();
            break;
        case 0xE: // SBC A,n
            instr = "SBC A," + ReadHex8();
            break;
        case 0xF: // RST 18h
            instr = "RST 18";
            break;
        }
        break;
    case 0xE:
        switch(op & 0x0F)
        {
        case 0x0: // RET PO
            instr = "RET PO";
            break;
        case 0x1: // POP HL
            instr = "POP HL";
            break;
        case 0x2: // JP PO,nn
            instr = "JP PO," + ReadHex16();
            break;
        case 0x3: // EX (SP),HL
            instr = "EX (SP),HL";
            break;
        case 0x4: // CALL PO,nn
            instr = "CALL PO," + ReadHex16();
            break;
        case 0x5: // PUSH_HL
            instr = "PUSH HL";
            break;
        case 0x6: // AND n
            instr = "AND " + ReadHex8();
            break;
        case 0x7: // RST 20h
            instr = "RST 20";
            break;
        case 0x8: // RET PE
            instr = "RET PE";
            break;
        case 0x9: // JP (HL)
            instr = "JP (HL)";
            break;
        case 0xA: // JP PE,nn
            instr = "JP PE," + ReadHex16();
            break;
        case 0xB: // EX DE,HL
            instr = "EX DE,HL";
            break;
        case 0xC: // CALL PE,nn
            instr = "CALL PE," + ReadHex16();
            break;
        case 0xD: // MISC
            GetNextInstructionMisc();
            break;
        case 0xE: // XOR n
            instr = "XOR " + ReadHex8();
            break;
        case 0xF: // RST 28h
            instr = "RST 28";
            break;
        }
        break;
    case 0xF:
        switch(op & 0x0F)
        {
        case 0x0: // RET P
            instr = "RET P";
            break;
        case 0x1: // POP AF
            instr = "POP AF";
            break;
        case 0x2: // JP P,nn
            instr = "JP P," + ReadHex16();
            break;
        case 0x3: // DI
            instr = "DI";
            break;
        case 0x4: // CALL P,nn
            instr = "CALL P," + ReadHex16();
            break;
        case 0x5: // PUSH_AF
            instr = "PUSH AF";
            break;
        case 0x6: // OR n
            instr = "OR " + ReadHex8();
            break;
        case 0x7: // RST 30h
            instr = "RST 30";
            break;
        case 0x8: // RET M
            instr = "RET M";
            break;
        case 0x9: // LD SP,HL
            instr = "LD SP,HL";
            break;
        case 0xA: // JP M,nn
            instr = "JP M," + ReadHex16();
            break;
        case 0xB: // EI
            instr = "EI";
            break;
        case 0xC: // CALL M,nn
            instr = "CALL M," + ReadHex16();
            break;
        case 0xD: // IY op
            idx = "IY";
            GetNextInstructionIDX();
            break;
        case 0xE: // CP n
            instr = "CP " + ReadHex8();
            break;
        case 0xF: // RST 38h
            instr = "RST 38";
            break;
        }
        break;
    }
}

void Disassembler::GetNextInstructionCB()
{
    op = ReadNext();
    instr = CB_INSTR[op / 8] + CB_OPERAND[op % 8];
}

void Disassembler::GetNextInstructionIDXCB()
{
    d = ReadHex8();
    op = ReadNext();
    instr = CB_INSTR[op / 8] + GetRelativeIndex();
    if (CB_OPERAND[op % 8][0] != '(')
        instr += "," +CB_OPERAND[op % 8];
}

void Disassembler::GetNextInstructionIDX()
{
    op = ReadNext();
    switch(op >> 4)
    {
    case 0x0:
        switch(op & 0x0F)
        {
        case 0x4: // INC B
        case 0x5: // DEC B
        case 0x6: // LD B,n
        case 0xC: // INC C
        case 0xD: // DEC C
        case 0xE: // LD C,n
            GetNextInstructionBasic();
            break;
        case 0x9: // ADD IDX,BC
            instr = "ADD " + idx + ",BC";
            break;
        }
        break;
    case 0x1:
        switch(op & 0x0F)
        {
        case 0x4: // INC D
        case 0x5: // DEC D
        case 0x6: // LD D,n
        case 0xC: // INC E
        case 0xD: // DEC E
        case 0xE: // LD E,n
            GetNextInstructionBasic();
            break;
        case 0x9: // ADD IDX,DE
            instr = "ADD " + idx + ",DE";
            break;
        }
        break;
    case 0x2:
        switch(op & 0x0F)
        {
        case 0x1: // LD IX,nn
            instr = "LD " + idx + "," + ReadHex16();
            break;
        case 0x2: // LD (nn),IX
            instr = "LD (" + ReadHex16() + ")," + idx;
            break;
        case 0x3: // INC IX
            instr = "INC " + idx;
            break;
        case 0x4: // INC IXH
            instr = "INC " + idx + "H";
            break;
        case 0x5: // DEC IXH
            instr = "DEC " + idx + "H";
            break;
        case 0x6: // LD IXH,n
            instr = "LD " + idx + "H," + ReadHex8();
            break;
        case 0x9: // ADD IDX,IDX
            instr = "ADD " + idx + "," + idx;
            break;
        case 0xA: // LD IX,(nn)
            instr = "LD " + idx + ",(" + ReadHex16() + ")";
            break;
        case 0xB: // DEC IX
            instr = "DEC " + idx;
            break;
        case 0xC: // INC IXL
            instr = "INC " + idx + "L";
            break;
        case 0xD: // DEC IXL
            instr = "DEC " + idx + "L";
            break;
        case 0xE: // LD IXL,n
            instr = "LD " + idx + "L," + ReadHex8();
            break;
        }
        break;
    case 0x3:
        switch(op & 0x0F)
        {
        case 0x4: // INC (IX+d)
        case 0x5: // DEC (IX+d)
        case 0x6: // LD (IX+d),n
            GetNextInstructionIDX2();
            break;
        case 0x9: // ADD IX,SP
            instr = "ADD " + idx + ",SP";
            break;
        case 0xC: //
        case 0xD: //
        case 0xE: //
            GetNextInstructionBasic();
            break;
        }
        break;
    case 0x4:
        switch(op & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            GetNextInstructionBasic();
            break;
        case 0x4: // LD B,IDX.H
            instr = "LD B," + idx + "H";
            break;
        case 0x5: // LD B,IDX.L
            instr = "LD B," + idx + "L";
            break;
        case 0x6: // LD B,(IDX+d)
        case 0xE: // LD C,(IX+n)
            GetNextInstructionIDX2();
            break;
        case 0xC: // LD C,IDX.H
            instr = "LD C," + idx + "H";
            break;
        case 0xD: // LD C,IDX.L
            instr = "LD C," + idx + "L";
            break;
        }
        break;
    case 0x5:
        switch(op & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            GetNextInstructionBasic();
            break;
        case 0x4: // LD D,IDX.H
            instr = "LD D," + idx + "H";
            break;
        case 0x5: // LD D,IDX.L
            instr = "LD D," + idx + "L";
            break;
        case 0x6: // LD D,(IDX+d)
        case 0xE: // LD E,(IX+n)
            GetNextInstructionIDX2();
            break;
        case 0xC: // LD E,IDX.H
            instr = "LD E," + idx + "H";
            break;
        case 0xD: // LD E,IDX.L
            instr = "LD E," + idx + "L";
            break;
        }
        break;
    case 0x6:
        switch(op & 0x0F)
        {
        case 0x0: // LD IDX.H,B
            instr = "LD " + idx + "H,B";
            break;
        case 0x1: // LD IDX.H,C
            instr = "LD " + idx + "H,C";
            break;
        case 0x2: // LD IDX.H,D
            instr = "LD " + idx + "H,D";
            break;
        case 0x3: // LD IDX.H,E
            instr = "LD " + idx + "H,E";
            break;
        case 0x4: // LD IDX.H,IDX.H
            instr = "LD " + idx + "H," + idx +"H";
            break;
        case 0x5: // LD IDX.H,IDX.L
            instr = "LD " + idx + "H," + idx +"L";
            break;
        case 0x6: // LD H,(IDX+d)
        case 0xE: // LD L,(IX+n)
            GetNextInstructionIDX2();
            break;
        case 0x7: // LD IDX.H,A
            instr = "LD " + idx + "H,A";
            break;
        case 0x8: // LD IDX.L,B
            instr = "LD " + idx + "L,B";
            break;
        case 0x9: // LD IDX.L,C
            instr = "LD " + idx + "L,C";
            break;
        case 0xA: // LD IDX.L,D
            instr = "LD " + idx + "L,D";
            break;
        case 0xB: // LD IDX.L,E
            instr = "LD " + idx + "L,E";
            break;
        case 0xC: // LD IDX.L,IDX.H
            instr = "LD " + idx + "L," + idx +"H";
            break;
        case 0xD: // LD IDX.L,IDX.L
            instr = "LD " + idx + "L," + idx +"L";
            break;
        case 0xF: // LD IDX.L,A
            instr = "LD " + idx + "L,A";
            break;
        }
        break;
    case 0x7:
        switch(op & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x4: //
        case 0x5: //
        case 0x7: //
        case 0xE: // LD A,(IX+d)
            GetNextInstructionIDX2();
            break;
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            GetNextInstructionBasic();
            break;
        case 0xC: // LD A,IDX.H
            instr = "LD A," + idx + "H";
            break;
        case 0xD: // LD A,IDX.L
            instr = "LD A," + idx + "L";
            break;
        }
        break;
    case 0x8:
        switch(op & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            GetNextInstructionBasic();
            break;
        case 0x6: //
        case 0xE: //
            GetNextInstructionIDX2();
            break;
        case 0x4: // ADD IDX.H
            instr = "ADD " + idx + "H";
            break;
        case 0x5: // ADD IDX.L
            instr = "ADD " + idx + "L";
            break;
        case 0xC: // ADC IDX.H
            instr = "ADC " + idx + "H";
            break;
        case 0xD: // ADC IDX.L
            instr = "ADC " + idx + "L";
            break;
        }
        break;
    case 0x9:
        switch(op & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            GetNextInstructionBasic();
            break;
        case 0x6: //
        case 0xE: //
            GetNextInstructionIDX2();
            break;
        case 0x4: // SUB IDX.H
            instr = "SUB " + idx + "H";
            break;
        case 0x5: // SUB IDX.L
            instr = "SUB " + idx + "L";
            break;
        case 0xC: // SBC IDX.H
            instr = "SBC " + idx + "H";
            break;
        case 0xD: // SBC IDX.L
            instr = "SBC " + idx + "L";
            break;
        }
        break;
    case 0xA:
        switch(op & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            GetNextInstructionBasic();
            break;
        case 0x6: //
        case 0xE: //
            GetNextInstructionIDX2();
            break;
        case 0x4: // AND IDX.H
            instr = "AND " + idx + "H";
            break;
        case 0x5: // AND IDX.L
            instr = "AND " + idx + "L";
            break;
        case 0xC: // XOR IDX.H
            instr = "XOR " + idx + "H";
            break;
        case 0xD: // XOR IDX.L
            instr = "XOR " + idx + "L";
            break;
        }
        break;
    case 0xB:
        switch(op & 0x0F)
        {
        case 0x0: //
        case 0x1: //
        case 0x2: //
        case 0x3: //
        case 0x7: //
        case 0x8: //
        case 0x9: //
        case 0xA: //
        case 0xB: //
        case 0xF: //
            GetNextInstructionBasic();
            break;
        case 0x6: //
        case 0xE: //
            GetNextInstructionIDX2();
            break;
        case 0x4: // OR IDX.H
            instr = "OR " + idx + "H";
            break;
        case 0x5: // OR IDX.L
            instr = "OR " + idx + "L";
            break;
        case 0xC: // CP IDX.H
            instr = "CP " + idx + "H";
            break;
        case 0xD: // CP IDX.L
            instr = "CP " + idx + "L";
            break;
        }
        break;
    case 0xC:
        switch(op & 0x0F)
        {
        case 0xB: // IDX BIT OP
            GetNextInstructionIDXCB();
            break;
        }
        break;
    case 0xE:
        switch(op & 0x0F)
        {
        case 0x1: // POP IDX
            instr = "POP " + idx;
            break;
        case 0x3: // EX SP,(IDX)
            instr = "EX SP,(" + idx + ")";
            break;
        case 0x5: // PUSH IDX
            instr = "PUSH " + idx;
            break;
        case 0x9: // JP (IDX)
            instr = "JP (" + idx + ")";
            break;
        }
        break;
    case 0xF:
        switch(op & 0x0F)
        {
        case 0x9: // LD SP,IX
            instr = "LD SP," + idx;
            break;
        }
        break;
    }
}

string Disassembler::GetRelativeIndex()
{
    return "(" + idx + "+" + d + ")";
}

void Disassembler::GetNextInstructionIDX2()
{
    d = ReadHex8();
    switch(op >> 4)
    {
    case 0x3:
        switch(op & 0x0F)
        {
        case 0x4:
            instr = "INC " + GetRelativeIndex();
            break;
        case 0x5:
            instr = "DEC " + GetRelativeIndex();
            break;
        case 0x6:
            instr = "LD " + GetRelativeIndex() + "," + ReadHex8();
            break;
        }
        break;
    case 0x4:
        switch(op & 0x0F)
        {
        case 0x6:
            instr = "LD B," + GetRelativeIndex();
            break;
        case 0xE:
            instr = "LD C," + GetRelativeIndex();
            break;
        }
        break;
    case 0x5:
        switch(op & 0x0F)
        {
        case 0x6:
            instr = "LD D," + GetRelativeIndex();
            break;
        case 0xE:
            instr = "LD E," + GetRelativeIndex();
            break;
        }
        break;
    case 0x6:
        switch(op & 0x0F)
        {
        case 0x6:
            instr = "LD H," + GetRelativeIndex();
            break;
        case 0xE:
            instr = "LD L," + GetRelativeIndex();
            break;
        }
        break;
    case 0x7:
        switch(op & 0x0F)
        {
        case 0x0: // LD (IDX+d),B
            instr = "LD " + GetRelativeIndex() + ",B";
            break;
        case 0x1: // LD (IDX+d),C
            instr = "LD " + GetRelativeIndex() + ",C";
            break;
        case 0x2: // LD (IDX+d),D
            instr = "LD " + GetRelativeIndex() + ",D";
            break;
        case 0x3: // LD (IDX+d),E
            instr = "LD " + GetRelativeIndex() + ",E";
            break;
        case 0x4: // LD (IDX+d),H
            instr = "LD " + GetRelativeIndex() + ",H";
            break;
        case 0x5: // LD (IDX+d),L
            instr = "LD " + GetRelativeIndex() + ",L";
            break;
        case 0x7: // LD (IDX+d),A
            instr = "LD " + GetRelativeIndex() + ",A";
            break;
        case 0xE: // LD A,(IX+d)
            instr = "LD A," + GetRelativeIndex();
            break;
        }
        break;
    case 0x8:
        switch(op & 0x0F)
        {
        case 0x6:
            instr = "ADD " + GetRelativeIndex();
            break;
        case 0xE:
            instr = "ADC " + GetRelativeIndex();
            break;
        }
        break;
    case 0x9:
        switch(op & 0x0F)
        {
        case 0x6:
            instr = "SUB " + GetRelativeIndex();
            break;
        case 0xE:
            instr = "SBC " + GetRelativeIndex();
            break;
        }
        break;
    case 0xA:
        switch(op & 0x0F)
        {
        case 0x6:
            instr = "AND " + GetRelativeIndex();
            break;
        case 0xE:
            instr = "XOR " + GetRelativeIndex();
            break;
        }
        break;
    case 0xB:
        switch(op & 0x0F)
        {
        case 0x6:
            instr = "OR " + GetRelativeIndex();
            break;
        case 0xE:
            instr = "CP " + GetRelativeIndex();
            break;
        }
        break;
    }
}

void Disassembler::GetNextInstructionMisc()
{
    op = ReadNext();
    switch(op >> 4)
    {
    case 0x4:
        switch(op & 0x0F)
        {
        case 0x0: // IN B,(C)
            instr = "IN B,(C)";
            break;
        case 0x1: // OUT (C),B
            instr = "OUT (C),B";
            break;
        case 0x2: // SBC HL,BC
            instr = "SBC HL,BC";
            break;
        case 0x3: // LD (nn),BC
            instr = "LD (" + ReadHex16() + "),BC";
            break;
        case 0x4: // NEG
            instr = "NEG";
            break;
        case 0x5: // RETN
            instr = "RETN";
            break;
        case 0x6: // IM 0
            instr = "IM 0";
            break;
        case 0x7: // LD I,A
            instr = "LD I,A";
            break;
        case 0x8: // IN C,(C)
            instr = "IN C,(C)";
            break;
        case 0x9: // OUT (C),C
            instr = "OUT (C),C";
            break;
        case 0xA: // ADC HL,BC
            instr = "ADC HL,BC";
            break;
        case 0xB: // LD BC,(nn)
            instr = "LD BC,(" + ReadHex16() + ")";
            break;
        case 0xC: // NEG
            instr = "NEG";
            break;
        case 0xD: // RETI
            instr = "RETI";
            break;
        case 0xE: // IM 0
            instr = "IM 0";
            break;
        case 0xF: // LD R,A
            instr = "LD R,A";
            break;
        }
        break;
    case 0x5:
        switch(op & 0x0F)
        {
        case 0x0: // IN D,(C)
            instr = "IN E,(C)";
            break;
        case 0x1: // OUT (C),D
            instr = "OUT (C),D";
            break;
        case 0x2: // SBC HL,DE
            instr = "SBC HL,DE";
            break;
        case 0x3: // LD (nn),DE
            instr = "LD (" + ReadHex16() + "),DE";
            break;
        case 0x4: // NEG
            instr = "NEG";
            break;
        case 0x5: // RETN
            instr = "RETN";
            break;
        case 0x6: // IM 1
            instr = "IM 1";
            break;
        case 0x7: // LD A,I
            instr = "LD A,I";
            break;
        case 0x8: // IN E,(C)
            instr = "IN E,(C)";
            break;
        case 0x9: // OUT (C),E
            instr = "OUT (C),E";
            break;
        case 0xA: // ADC HL,DE
            instr = "ADC HL,DE";
            break;
        case 0xB: // LD DE,(nn)
            instr = "LD DE,(" + ReadHex16() + ")";
            break;
        case 0xC: // NEG
            instr = "NEG";
            break;
        case 0xD: // RETN
            instr = "RETN";
            break;
        case 0xE: // IM 2
            instr = "IM 2";
            break;
        case 0xF: // LD A,R
            instr = "LD A,R";
            break;
        }
        break;
    case 0x6:
        switch(op & 0x0F)
        {
        case 0x0: // IN H,(C)
            instr = "IN H,(C)";
            break;
        case 0x1: // OUT (C),H
            instr = "OUT (C),H";
            break;
        case 0x2: // SBC HL,HL
            instr = "SBC HL,HL";
            break;
        case 0x3: // LD (nn),HL
            instr = "LD (" + ReadHex16() + "),HL";
            break;
        case 0x4: // NEG
            instr = "NEG";
            break;
        case 0x5: // RETN
            instr = "RETN";
            break;
        case 0x6: // IM 0
            instr = "IM 0";
            break;
        case 0x7: // RRD
            instr = "RRD";
            break;
        case 0x8: // IN L,(C)
            instr = "IN L,(C)";
            break;
        case 0x9: // OUT (C),L
            instr = "OUT (C),L";
            break;
        case 0xA: // ADC HL,HL
            instr = "ADC HL,HL";
            break;
        case 0xB: // LD HL,(nn)
            instr = "LD HL,(" + ReadHex16() + ")";
            break;
        case 0xC: // NEG
            instr = "NEG";
            break;
        case 0xD: // RETN
            instr = "RETN";
            break;
        case 0xE: // IM 0
            instr = "IM 0";
            break;
        case 0xF: // RLD
            instr = "RLD";
            break;
        }
        break;
    case 0x7:
        switch(op & 0x0F)
        {
        case 0x0: // IN (C) Flags only
            instr = "IN (C)";
            break;
        case 0x1: // OUT (C),0
            instr = "OUT (C),0";
            break;
        case 0x2: // SBC HL,SP
            instr = "SBC HL,SP";
            break;
        case 0x3: // LD (nn),SP
            instr = "LD (" + ReadHex16() + "),SP";
            break;
        case 0x4: // NEG
            instr = "NEG";
            break;
        case 0x5: // RETN
            instr = "RETN";
            break;
        case 0x6: // IM 1
            instr = "IM 1";
            break;
        case 0x7: // LD I,A
            instr = "LD I,A";
            break;
        case 0x8: // IN A,(C)
            instr = "IN A,(C)";
            break;
        case 0x9: // OUT (C),A
            instr = "OUT (C),A";
            break;
        case 0xA: // ADC HL,SP
            instr = "ADC HL,SP";
            break;
        case 0xB: // LD SP,(nn)
            instr = "LD SP,(" + ReadHex16() + ")";
            break;
        case 0xC: // NEG
            instr = "NEG";
            break;
        case 0xD: // RETN
            instr = "RETN";
            break;
        case 0xE: // IM 2
            instr = "IM 2";
            break;
        }
        break;
    case 0xA:
        switch(op & 0x0F)
        {
        case 0x0: // LDI
            instr = "LDI";
            break;
        case 0x1: // CPI
            instr = "CPI";
            break;
        case 0x2: // INI
            instr = "INI";
            break;
        case 0x3: // OUTI
            instr = "OUTI";
            break;
        case 0x8: // LDD
            instr = "LDD";
            break;
        case 0x9: // CPD
            instr = "CPD";
            break;
        case 0xA: // IND
            instr = "IND";
            break;
        case 0xB: // OUTD
            instr = "OUTD";
            break;
        }
        break;
    case 0xB:
        switch(op & 0x0F)
        {
        case 0x0: // LDIR
            instr = "LDIR";
            break;
        case 0x1: // CPIR
            instr = "CPIR";
            break;
        case 0x2: // INIR
            instr = "INIR";
            break;
        case 0x3: // OUTIR
            instr = "OUTIR";
            break;
        case 0x8: // LDDR
            instr = "LDDR";
            break;
        case 0x9: // CPDR
            instr = "CPDR";
            break;
        case 0xA: // INDR
            instr = "INDR";
            break;
        case 0xB: // OUTDR
            instr = "OUTDR";
            break;
        }
        break;
    }
}
