#include "Disassembler.h"
#include "CPC.h"
#include <cstdio>
#include <map>
#include <string>

using std::string;

int Disassembler::addr;

namespace {

// Labels map + helpers to add entries from Disassembler::Init.
std::map<word, string> labels;

void addLabel(word a, const char *text)
{
    labels.insert_or_assign(a, string(text));
}

// Standard Z80 decode tables.
// Opcode bitfield layout: [x(2) | y(3) | z(3)], with p = y>>1, q = y&1.
constexpr const char *r8[8]     = {"B","C","D","E","H","L","(HL)","A"};
constexpr const char *rp[4]     = {"BC","DE","HL","SP"};
constexpr const char *rp2[4]    = {"BC","DE","HL","AF"};
constexpr const char *cc[8]     = {"NZ","Z","NC","C","PO","PE","P","M"};
constexpr const char *aluReg[8] = {"ADD ","ADC ","SUB ","SBC ","AND ","XOR ","OR ","CP "};
constexpr const char *aluImm[8] = {"ADD A,","ADC A,","SUB ","SBC A,","AND ","XOR ","OR ","CP "};
constexpr const char *rotOp[8]  = {"RLC","RRC","RL","RR","SLA","SRA","SLL","SRL"};
constexpr const char *imOp[8]   = {"0","0","1","2","0","0","1","2"};
constexpr const char *blkOp[4][4] = {
    {"LDI",  "CPI",  "INI",  "OUTI" },
    {"LDD",  "CPD",  "IND",  "OUTD" },
    {"LDIR", "CPIR", "INIR", "OUTIR"},
    {"LDDR", "CPDR", "INDR", "OUTDR"}
};

struct State {
    BYTE length = 0;
    BYTE op = 0;
    string instr = "??";
    string bytes;
    const char *idx = nullptr;  // "IX"/"IY" under DD/FD prefix; nullptr otherwise.
};

BYTE readNext(State &s)
{
    BYTE b = CPC::GetByteAt(Disassembler::addr);
    char buf[8];
    snprintf(buf, sizeof(buf), "%02hhX ", (unsigned char)b);
    s.bytes += buf;
    s.length++;
    Disassembler::addr++;
    return b;
}

string readHex8(State &s)
{
    BYTE v = readNext(s);
    char buf[8];
    snprintf(buf, sizeof(buf), "&%02hhX", (unsigned char)v);
    return buf;
}

string readHex16(State &s)
{
    int lo = readNext(s);
    int hi = readNext(s);
    word w = (word)(lo + hi * 256);
    auto pos = labels.find(w);
    if (pos != labels.end()) return pos->second;
    char buf[8];
    snprintf(buf, sizeof(buf), "&%04X", w);
    return buf;
}

string readRel(State &s)
{
    sbyte off = (sbyte)readNext(s);
    char buf[8];
    snprintf(buf, sizeof(buf), "&%04X", Disassembler::addr + off);
    return buf;
}

// Substitute r8[i] under IDX prefix.
//   i==4 → "IXH"/"IYH", i==5 → "IXL"/"IYL", i==6 → "(IX+d)" (consumes one byte).
// Caller that has already mapped (HL)-side to (IX+d) separately should NOT call with
// i==6; this helper assumes the (HL) slot can legitimately be replaced here.
string rSubIdx(State &s, BYTE i)
{
    if (!s.idx) return r8[i];
    switch (i) {
    case 4: return string(s.idx) + "H";
    case 5: return string(s.idx) + "L";
    case 6: return "(" + string(s.idx) + "+" + readHex8(s) + ")";
    default: return r8[i];
    }
}

// rp[] with HL→IX/IY when under IDX prefix.
string rpIdx(State &s, BYTE p)
{
    if (s.idx && p == 2) return s.idx;
    return rp[p];
}

// rp2[] with HL→IX/IY when under IDX prefix.
string rp2Idx(State &s, BYTE p)
{
    if (s.idx && p == 2) return s.idx;
    return rp2[p];
}

// Forward decls.
void decodeMain(State &s);
void decodeCB(State &s);
void decodeED(State &s);
void decodeIDX(State &s, const char *which);
void decodeIDXCB(State &s);

void decodeMain(State &s)
{
    BYTE op = s.op;
    BYTE x = op >> 6;
    BYTE y = (op >> 3) & 7;
    BYTE z = op & 7;
    BYTE p = y >> 1;
    BYTE q = y & 1;

    switch (x) {
    case 0:
        switch (z) {
        case 0:  // Relative jumps / EX AF,AF' / NOP
            switch (y) {
            case 0: s.instr = "NOP"; break;
            case 1: s.instr = "EX AF,AF'"; break;
            case 2: s.instr = "DJNZ " + readRel(s); break;
            case 3: s.instr = "JR " + readRel(s); break;
            default: s.instr = string("JR ") + cc[y - 4] + "," + readRel(s); break;
            }
            break;
        case 1:  // 16-bit immediate / ADD HL,rp
            if (q == 0) s.instr = "LD " + rpIdx(s, p) + "," + readHex16(s);
            else        s.instr = "ADD " + rpIdx(s, 2) + "," + rpIdx(s, p);
            break;
        case 2:  // Indirect loads
            switch (y) {
            case 0: s.instr = "LD (BC),A"; break;
            case 1: s.instr = "LD A,(BC)"; break;
            case 2: s.instr = "LD (DE),A"; break;
            case 3: s.instr = "LD A,(DE)"; break;
            case 4: s.instr = "LD (" + readHex16(s) + ")," + rpIdx(s, 2); break;
            case 5: s.instr = "LD " + rpIdx(s, 2) + ",(" + readHex16(s) + ")"; break;
            case 6: s.instr = "LD (" + readHex16(s) + "),A"; break;
            case 7: s.instr = "LD A,(" + readHex16(s) + ")"; break;
            }
            break;
        case 3:  // INC/DEC rp
            if (q == 0) s.instr = "INC " + rpIdx(s, p);
            else        s.instr = "DEC " + rpIdx(s, p);
            break;
        case 4:  // INC r
            s.instr = "INC " + rSubIdx(s, y);
            break;
        case 5:  // DEC r
            s.instr = "DEC " + rSubIdx(s, y);
            break;
        case 6: { // LD r,n  (when y==6 and idx set, consumes disp THEN immediate)
            string lhs = rSubIdx(s, y);
            s.instr = "LD " + lhs + "," + readHex8(s);
            break;
        }
        case 7:
            switch (y) {
            case 0: s.instr = "RLCA"; break;
            case 1: s.instr = "RRCA"; break;
            case 2: s.instr = "RLA";  break;
            case 3: s.instr = "RRA";  break;
            case 4: s.instr = "DAA";  break;
            case 5: s.instr = "CPL";  break;
            case 6: s.instr = "SCF";  break;
            case 7: s.instr = "CCF";  break;
            }
            break;
        }
        break;

    case 1: { // LD r,r' (0x40..0x7F) with 0x76 → HALT
        if (y == 6 && z == 6) { s.instr = "HALT"; break; }
        string dst, src;
        if (s.idx && (y == 6 || z == 6)) {
            // The (HL) side becomes (IX+d); the other side keeps H/L as literal.
            string mem = "(" + string(s.idx) + "+" + readHex8(s) + ")";
            if (y == 6) { dst = mem;    src = r8[z]; }
            else        { dst = r8[y];  src = mem;   }
        } else {
            dst = rSubIdx(s, y);
            src = rSubIdx(s, z);
        }
        s.instr = "LD " + dst + "," + src;
        break;
    }

    case 2: { // ALU r (0x80..0xBF)
        string operand;
        if (z == 6 && s.idx) {
            operand = "(" + string(s.idx) + "+" + readHex8(s) + ")";
        } else {
            operand = rSubIdx(s, z);
        }
        s.instr = aluReg[y] + operand;
        break;
    }

    case 3:
        switch (z) {
        case 0:
            s.instr = string("RET ") + cc[y];
            break;
        case 1:
            if (q == 0) s.instr = "POP " + rp2Idx(s, p);
            else switch (p) {
                case 0: s.instr = "RET"; break;
                case 1: s.instr = "EXX"; break;
                case 2: s.instr = string("JP (") + (s.idx ? s.idx : "HL") + ")"; break;
                case 3: s.instr = string("LD SP,") + (s.idx ? s.idx : "HL"); break;
            }
            break;
        case 2:
            s.instr = string("JP ") + cc[y] + "," + readHex16(s);
            break;
        case 3:
            switch (y) {
            case 0: s.instr = "JP " + readHex16(s); break;
            case 1: decodeCB(s); break;
            case 2: s.instr = "OUT " + readHex8(s) + ",A"; break;
            case 3: s.instr = "IN A," + readHex8(s); break;
            case 4: s.instr = string("EX (SP),") + (s.idx ? s.idx : "HL"); break;
            case 5: s.instr = "EX DE,HL"; break;
            case 6: s.instr = "DI"; break;
            case 7: s.instr = "EI"; break;
            }
            break;
        case 4:
            s.instr = string("CALL ") + cc[y] + "," + readHex16(s);
            break;
        case 5:
            if (q == 0) s.instr = "PUSH " + rp2Idx(s, p);
            else switch (p) {
                case 0: s.instr = "CALL " + readHex16(s); break;
                case 1: decodeIDX(s, "IX"); break;
                case 2: decodeED(s); break;
                case 3: decodeIDX(s, "IY"); break;
            }
            break;
        case 6:
            s.instr = aluImm[y] + readHex8(s);
            break;
        case 7: {
            char buf[8];
            snprintf(buf, sizeof(buf), "RST %02X", y * 8);
            s.instr = buf;
            break;
        }
        }
        break;
    }
}

void decodeCB(State &s)
{
    if (s.idx) { decodeIDXCB(s); return; }
    BYTE op = readNext(s);
    BYTE x = op >> 6;
    BYTE y = (op >> 3) & 7;
    BYTE z = op & 7;
    char buf[8];
    switch (x) {
    case 0: s.instr = string(rotOp[y]) + " " + r8[z]; break;
    case 1: snprintf(buf, sizeof(buf), "BIT %u,", y); s.instr = string(buf) + r8[z]; break;
    case 2: snprintf(buf, sizeof(buf), "RES %u,", y); s.instr = string(buf) + r8[z]; break;
    case 3: snprintf(buf, sizeof(buf), "SET %u,", y); s.instr = string(buf) + r8[z]; break;
    }
}

void decodeIDXCB(State &s)
{
    // DD CB d op  (displacement comes BEFORE the opcode byte in encoding).
    string d = readHex8(s);
    BYTE op = readNext(s);
    BYTE x = op >> 6;
    BYTE y = (op >> 3) & 7;
    BYTE z = op & 7;
    string target = "(" + string(s.idx) + "+" + d + ")";
    string mnem;
    char buf[8];
    switch (x) {
    case 0: mnem = string(rotOp[y]) + " "; break;
    case 1: snprintf(buf, sizeof(buf), "BIT %u,", y); mnem = buf; break;
    case 2: snprintf(buf, sizeof(buf), "RES %u,", y); mnem = buf; break;
    case 3: snprintf(buf, sizeof(buf), "SET %u,", y); mnem = buf; break;
    }
    s.instr = mnem + target;
    // Undocumented: when z != 6 the result is ALSO stored in r8[z].
    // Preserve legacy format: append ",r" for every non-(HL) slot regardless of x.
    if (z != 6) s.instr += "," + string(r8[z]);
}

void decodeED(State &s)
{
    BYTE op = readNext(s);
    BYTE x = op >> 6;
    BYTE y = (op >> 3) & 7;
    BYTE z = op & 7;
    BYTE p = y >> 1;
    BYTE q = y & 1;
    s.instr = "??";
    switch (x) {
    case 1:
        switch (z) {
        case 0:
            if (y == 6) s.instr = "IN (C)";
            else        s.instr = string("IN ") + r8[y] + ",(C)";
            break;
        case 1:
            if (y == 6) s.instr = "OUT (C),0";
            else        s.instr = string("OUT (C),") + r8[y];
            break;
        case 2:
            if (q == 0) s.instr = string("SBC HL,") + rp[p];
            else        s.instr = string("ADC HL,") + rp[p];
            break;
        case 3:
            if (q == 0) s.instr = "LD (" + readHex16(s) + ")," + rp[p];
            else        s.instr = string("LD ") + rp[p] + ",(" + readHex16(s) + ")";
            break;
        case 4: s.instr = "NEG"; break;
        case 5: s.instr = (y == 1) ? "RETI" : "RETN"; break;
        case 6: s.instr = string("IM ") + imOp[y]; break;
        case 7:
            switch (y) {
            case 0: s.instr = "LD I,A"; break;
            case 1: s.instr = "LD R,A"; break;
            case 2: s.instr = "LD A,I"; break;
            case 3: s.instr = "LD A,R"; break;
            case 4: s.instr = "RRD";    break;
            case 5: s.instr = "RLD";    break;
            // y==6 and y==7 are NOPs (undocumented): leave instr = "??".
            }
            break;
        }
        break;
    case 2:
        // Block instructions: y in [4..7], z in [0..3].
        if (y >= 4 && z <= 3) s.instr = blkOp[y - 4][z];
        break;
    }
}

void decodeIDX(State &s, const char *which)
{
    s.idx = which;
    s.op = readNext(s);
    decodeMain(s);
}

}  // namespace

void Disassembler::Init()
{
    // BIOS routines
    addLabel(0xBB09, "KMReadChar");
    addLabel(0xBB0C, "KMCharReturn");
    addLabel(0xBB1E, "KMTestKey");
    addLabel(0xBB5A, "TXTOutput");
    addLabel(0xBB66, "TXTWinEnable");
    addLabel(0xBB75, "TXTSetCursor");
    addLabel(0xBB90, "TXTSetPen");
    addLabel(0xBB96, "TXTSetPaper");
    addLabel(0x8ECA, "LineAddrTable");
    addLabel(0xBCAA, "SoundQueue");

    // AMSDOS routines
    addLabel(0xC7C7, "FDC_SenseINT");
    addLabel(0xC91C, "FDC_ReadResult");
    addLabel(0xC95C, "FDC_SendByte");
}

void Disassembler::SetPoint(word address)
{
    addr = address;
}

void Disassembler::GetNextInstruction(BYTE &instrLength, BYTE &opCode,
                                      string *label, string *addressOut,
                                      string *bytesOut, string *instrOut,
                                      int boundary)
{
    auto pos = labels.find((word)addr);
    *label = (pos != labels.end()) ? pos->second : string();

    char buf[16];
    int start = addr;
    snprintf(buf, sizeof(buf), "%04X", addr);
    *addressOut = buf;

    State s;
    s.op = readNext(s);
    opCode = s.op;
    decodeMain(s);

    if (addr > boundary)
    {
        // Instruction would cross an anchor — emit the first byte as data instead.
        addr = start + 1;
        BYTE b = CPC::GetByteAt((word)start);
        s.length = 1;
        snprintf(buf, sizeof(buf), "%02hhX ", (unsigned char)b);
        s.bytes = buf;
        snprintf(buf, sizeof(buf), "DB &%02hhX", (unsigned char)b);
        s.instr = buf;
    }

    *instrOut = s.instr;
    *bytesOut = s.bytes;
    instrLength = s.length;
}
