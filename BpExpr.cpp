#include "BpExpr.h"
#include "CPC.h"
#include "Z80.h"

#include <cctype>
#include <cstring>

namespace {

struct Lexer
{
    const char *p = nullptr;
    bool err = false;
    std::string errMsg;

    Lexer(const char *src) : p(src) {}

    void skipWS() { while (*p && std::isspace((unsigned char)*p)) p++; }
    bool eof() { skipWS(); return *p == 0; }

    bool peek(char c) { skipWS(); return *p == c; }
    bool peek2(char a, char b) { skipWS(); return p[0] == a && p[1] == b; }

    bool consume(char c) { skipWS(); if (*p == c) { p++; return true; } return false; }
    bool consume2(char a, char b) { skipWS(); if (p[0] == a && p[1] == b) { p += 2; return true; } return false; }

    bool isIdStart(char c) { return std::isalpha((unsigned char)c) || c == '_'; }
    bool isIdCont(char c)  { return std::isalnum((unsigned char)c) || c == '_' || c == '\''; }

    void error(const char *msg)
    {
        if (!err) { err = true; errMsg = msg; }
    }
};

static long parseOr(Lexer &L);
static long parsePrimary(Lexer &L);

static long readNumber(Lexer &L)
{
    L.skipWS();
    long val = 0;
    if (*L.p == '&' || *L.p == '$' || *L.p == '#')
    {
        char prefix = *L.p++;
        if (prefix == '&' && (*L.p == 'X' || *L.p == 'x'))
        {
            L.p++;
            if (*L.p != '0' && *L.p != '1') { L.error("expected binary digit"); return 0; }
            while (*L.p == '0' || *L.p == '1') { val = (val << 1) | (*L.p - '0'); L.p++; }
        }
        else
        {
            if (!std::isxdigit((unsigned char)*L.p)) { L.error("expected hex digit"); return 0; }
            while (std::isxdigit((unsigned char)*L.p))
            {
                int d = std::isdigit((unsigned char)*L.p) ? *L.p - '0'
                                                         : (std::toupper((unsigned char)*L.p) - 'A' + 10);
                val = val * 16 + d;
                L.p++;
            }
        }
        return val;
    }
    if (*L.p == '0' && (L.p[1] == 'x' || L.p[1] == 'X'))
    {
        L.p += 2;
        if (!std::isxdigit((unsigned char)*L.p)) { L.error("expected hex digit"); return 0; }
        while (std::isxdigit((unsigned char)*L.p))
        {
            int d = std::isdigit((unsigned char)*L.p) ? *L.p - '0'
                                                     : (std::toupper((unsigned char)*L.p) - 'A' + 10);
            val = val * 16 + d;
            L.p++;
        }
        return val;
    }
    if (std::isdigit((unsigned char)*L.p))
    {
        const char *start = L.p;
        while (std::isdigit((unsigned char)*L.p)) L.p++;
        const char *end = L.p;
        if (*L.p == 'H' || *L.p == 'h')
        {
            for (const char *q = start; q < end; q++)
            {
                if (!std::isxdigit((unsigned char)*q)) { L.error("bad hex literal"); return 0; }
                int d = std::isdigit((unsigned char)*q) ? *q - '0'
                                                       : (std::toupper((unsigned char)*q) - 'A' + 10);
                val = val * 16 + d;
            }
            L.p++;
        }
        else
        {
            for (const char *q = start; q < end; q++) val = val * 10 + (*q - '0');
        }
        return val;
    }
    L.error("expected number");
    return 0;
}

static std::string readIdent(Lexer &L)
{
    L.skipWS();
    if (!L.isIdStart(*L.p)) return {};
    const char *start = L.p;
    while (L.isIdCont(*L.p)) L.p++;
    std::string s(start, L.p);
    for (auto &c : s) c = (char)std::toupper((unsigned char)c);
    return s;
}

static long resolveIdent(Lexer &L, const std::string &id)
{
    Z80DebugState s = CPC::z80.GetDebugState();
    // 8-bit registers
    if (id == "A") return (s.AF >> 8) & 0xFF;
    if (id == "F") return s.AF & 0xFF;
    if (id == "B") return (s.BC >> 8) & 0xFF;
    if (id == "C") return s.BC & 0xFF;
    if (id == "D") return (s.DE >> 8) & 0xFF;
    if (id == "E") return s.DE & 0xFF;
    if (id == "H") return (s.HL >> 8) & 0xFF;
    if (id == "L") return s.HL & 0xFF;
    if (id == "IXH") return (s.IX >> 8) & 0xFF;
    if (id == "IXL") return s.IX & 0xFF;
    if (id == "IYH") return (s.IY >> 8) & 0xFF;
    if (id == "IYL") return s.IY & 0xFF;
    if (id == "R") return s.R;
    if (id == "I") return s.I;
    // 16-bit registers
    if (id == "AF") return s.AF;
    if (id == "BC") return s.BC;
    if (id == "DE") return s.DE;
    if (id == "HL") return s.HL;
    if (id == "IX") return s.IX;
    if (id == "IY") return s.IY;
    if (id == "PC") return s.PC;
    if (id == "SP") return s.SP;
    // Shadow registers (apostrophe form)
    if (id == "AF'") return s.AF_;
    if (id == "BC'") return s.BC_;
    if (id == "DE'") return s.DE_;
    if (id == "HL'") return s.HL_;
    // Flags as 0/1
    if (id == "FZ")  return s.fZ ? 1 : 0;
    if (id == "FC")  return s.fC ? 1 : 0;
    if (id == "FS")  return s.fS ? 1 : 0;
    if (id == "FN")  return s.fN ? 1 : 0;
    if (id == "FH")  return s.fH ? 1 : 0;
    if (id == "FP")  return s.fP ? 1 : 0;
    if (id == "FPV") return s.fP ? 1 : 0;
    L.error("unknown identifier");
    return 0;
}

static long parsePrimary(Lexer &L)
{
    L.skipWS();
    if (L.consume('('))
    {
        long v = parseOr(L);
        if (!L.consume(')')) L.error("expected ')'");
        return v;
    }
    if (L.consume('['))
    {
        long addr = parseOr(L);
        if (!L.consume(']')) L.error("expected ']'");
        return CPC::GetByteAt((word)(addr & 0xFFFF));
    }
    if (L.consume('!')) return !parsePrimary(L);
    if (L.consume('~')) return ~parsePrimary(L);
    if (L.consume('-')) return -parsePrimary(L);
    if (L.consume('+')) return parsePrimary(L);

    L.skipWS();
    if (*L.p == '&' || *L.p == '$' || *L.p == '#') return readNumber(L);
    if (std::isdigit((unsigned char)*L.p)) return readNumber(L);
    if (L.isIdStart(*L.p))
    {
        std::string id = readIdent(L);
        return resolveIdent(L, id);
    }
    L.error("unexpected token");
    return 0;
}

static long parseMul(Lexer &L)
{
    long lhs = parsePrimary(L);
    for (;;)
    {
        L.skipWS();
        if (L.consume('*')) lhs = lhs * parsePrimary(L);
        else if (L.consume('/'))
        {
            long rhs = parsePrimary(L);
            lhs = rhs ? lhs / rhs : 0;
        }
        else if (L.consume('%'))
        {
            long rhs = parsePrimary(L);
            lhs = rhs ? lhs % rhs : 0;
        }
        else return lhs;
    }
}

static long parseAdd(Lexer &L)
{
    long lhs = parseMul(L);
    for (;;)
    {
        L.skipWS();
        if (L.consume('+')) lhs = lhs + parseMul(L);
        else if (L.consume('-')) lhs = lhs - parseMul(L);
        else return lhs;
    }
}

static long parseShift(Lexer &L)
{
    long lhs = parseAdd(L);
    for (;;)
    {
        L.skipWS();
        if (L.consume2('<', '<')) lhs = lhs << parseAdd(L);
        else if (L.consume2('>', '>')) lhs = lhs >> parseAdd(L);
        else return lhs;
    }
}

static long parseRel(Lexer &L)
{
    long lhs = parseShift(L);
    for (;;)
    {
        L.skipWS();
        if (L.consume2('<', '=')) lhs = lhs <= parseShift(L);
        else if (L.consume2('>', '=')) lhs = lhs >= parseShift(L);
        else if (L.peek('<') && !L.peek2('<', '<')) { L.p++; lhs = lhs < parseShift(L); }
        else if (L.peek('>') && !L.peek2('>', '>')) { L.p++; lhs = lhs > parseShift(L); }
        else return lhs;
    }
}

static long parseEq(Lexer &L)
{
    long lhs = parseRel(L);
    for (;;)
    {
        L.skipWS();
        if (L.consume2('=', '=')) lhs = (lhs == parseRel(L));
        else if (L.consume2('!', '=')) lhs = (lhs != parseRel(L));
        else return lhs;
    }
}

static long parseBitAnd(Lexer &L)
{
    long lhs = parseEq(L);
    for (;;)
    {
        L.skipWS();
        if (L.peek('&') && !L.peek2('&', '&')) { L.p++; lhs = lhs & parseEq(L); }
        else return lhs;
    }
}

static long parseBitXor(Lexer &L)
{
    long lhs = parseBitAnd(L);
    for (;;)
    {
        L.skipWS();
        if (L.consume('^')) lhs = lhs ^ parseBitAnd(L);
        else return lhs;
    }
}

static long parseBitOr(Lexer &L)
{
    long lhs = parseBitXor(L);
    for (;;)
    {
        L.skipWS();
        if (L.peek('|') && !L.peek2('|', '|')) { L.p++; lhs = lhs | parseBitXor(L); }
        else return lhs;
    }
}

static long parseAnd(Lexer &L)
{
    long lhs = parseBitOr(L);
    for (;;)
    {
        L.skipWS();
        if (L.consume2('&', '&')) lhs = (parseBitOr(L) && lhs);
        else return lhs;
    }
}

static long parseOr(Lexer &L)
{
    long lhs = parseAnd(L);
    for (;;)
    {
        L.skipWS();
        if (L.consume2('|', '|')) lhs = (parseAnd(L) || lhs);
        else return lhs;
    }
}

static bool isBlank(const std::string &s)
{
    for (char c : s) if (!std::isspace((unsigned char)c)) return false;
    return true;
}

} // namespace

bool BpExpr::eval(const std::string &expr)
{
    if (isBlank(expr)) return true;
    Lexer L(expr.c_str());
    long v = parseOr(L);
    L.skipWS();
    if (L.err || *L.p != 0) return true; // fail-safe: still break on bad expr
    return v != 0;
}

std::string BpExpr::validate(const std::string &expr)
{
    if (isBlank(expr)) return {};
    Lexer L(expr.c_str());
    (void)parseOr(L);
    L.skipWS();
    if (L.err) return L.errMsg;
    if (*L.p != 0) return std::string("trailing garbage near '") + *L.p + "'";
    return {};
}
