#include "Assembler.h"

#include <QChar>
#include <QHash>
#include <QSet>
#include <QStringList>
#include <QtGlobal>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace {

using i64 = qint64;

enum class Tk {
    End, Eol,
    Ident, Number, String, Char,
    Comma, Colon, LP, RP,
    Plus, Minus, Star, Slash, Percent,
    Amp, Pipe, Caret, Tilde, Bang,
    Shl, Shr,
    Eq, NotEq, Lt, Gt, LtEq, GtEq,
    LogAnd, LogOr,
    Dollar,
    Apos,
};

struct Token
{
    Tk t = Tk::End;
    QString text;
    i64 n = 0;
    int line = 1;
    int col = 1;
    QString source;
};

// ============================================================
// Lexer
// ============================================================

class Lexer
{
public:
    Lexer(const QString &src, const QString &sourceName = QString()) : s(src), src(sourceName) {}
    QVector<Token> all(QString *errorText, int *errorLine);

private:
    QChar peek(int o = 0) const { return (p + o < s.size()) ? s[p + o] : QChar(0); }
    void advance(int n = 1);
    bool atEnd() const { return p >= s.size(); }
    bool isIdStart(QChar c) const { return c.isLetter() || c == '_' || c == '.' || c == '@'; }
    bool isIdCont(QChar c) const { return c.isLetterOrNumber() || c == '_' || c == '.' || c == '?'; }
    Token makeNumber(i64 value, int line, int col);
    bool readHex(i64 &out);
    bool readBin(i64 &out);
    bool readDec(i64 &out);

    QString s;
    QString src;
    int p = 0;
    int line = 1;
    int col = 1;
};

void Lexer::advance(int n)
{
    for (int i = 0; i < n; i++)
    {
        if (p < s.size() && s[p] == '\n') { line++; col = 1; }
        else col++;
        p++;
    }
}

Token Lexer::makeNumber(i64 value, int ln, int cl)
{
    Token tk;
    tk.t = Tk::Number;
    tk.n = value;
    tk.line = ln;
    tk.col = cl;
    return tk;
}

bool Lexer::readHex(i64 &out)
{
    i64 v = 0;
    int count = 0;
    while (!atEnd())
    {
        QChar c = peek();
        int d = -1;
        if (c >= '0' && c <= '9') d = c.unicode() - '0';
        else if (c >= 'a' && c <= 'f') d = c.unicode() - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c.unicode() - 'A' + 10;
        else break;
        v = (v << 4) | d;
        advance();
        count++;
    }
    out = v;
    return count > 0;
}

bool Lexer::readBin(i64 &out)
{
    i64 v = 0;
    int count = 0;
    while (!atEnd())
    {
        QChar c = peek();
        if (c == '0' || c == '1') { v = (v << 1) | (c == '1' ? 1 : 0); advance(); count++; }
        else break;
    }
    out = v;
    return count > 0;
}

bool Lexer::readDec(i64 &out)
{
    i64 v = 0;
    int count = 0;
    while (!atEnd() && peek().isDigit())
    {
        v = v * 10 + (peek().unicode() - '0');
        advance();
        count++;
    }
    out = v;
    return count > 0;
}

QVector<Token> Lexer::all(QString *errorText, int *errorLine)
{
    QVector<Token> out;
    while (!atEnd())
    {
        QChar c = peek();

        if (c == '\n')
        {
            Token tk; tk.t = Tk::Eol; tk.line = line; tk.col = col;
            out.append(tk);
            advance();
            continue;
        }
        if (c == '\r') { advance(); continue; }
        if (c == ' ' || c == '\t') { advance(); continue; }
        if (c == ';')
        {
            while (!atEnd() && peek() != '\n') advance();
            continue;
        }

        int tkLine = line, tkCol = col;

        if (c == '&' || c == '#' || c == '$')
        {
            QChar nxt = peek(1);
            bool isHex = false;
            bool isBin = false;
            if (c == '&' && (nxt == 'X' || nxt == 'x')) { advance(2); isBin = true; }
            else if (c == '&') { advance(); isHex = true; }
            else if (c == '#') { advance(); isHex = true; }
            else if (c == '$' && ((nxt >= '0' && nxt <= '9') || (nxt >= 'A' && nxt <= 'F') || (nxt >= 'a' && nxt <= 'f')))
            { advance(); isHex = true; }

            if (isHex)
            {
                i64 v = 0;
                if (!readHex(v))
                {
                    if (errorText) *errorText = "Invalid hex literal";
                    if (errorLine) *errorLine = tkLine;
                    return out;
                }
                out.append(makeNumber(v, tkLine, tkCol));
                continue;
            }
            if (isBin)
            {
                i64 v = 0;
                if (!readBin(v))
                {
                    if (errorText) *errorText = "Invalid binary literal";
                    if (errorLine) *errorLine = tkLine;
                    return out;
                }
                out.append(makeNumber(v, tkLine, tkCol));
                continue;
            }

            if (c == '$')
            {
                advance();
                Token tk; tk.t = Tk::Dollar; tk.line = tkLine; tk.col = tkCol;
                out.append(tk);
                continue;
            }
            advance();
            continue;
        }

        if (c == '%')
        {
            QChar nxt = peek(1);
            if (nxt == '0' || nxt == '1')
            {
                advance();
                i64 v = 0;
                if (!readBin(v))
                {
                    if (errorText) *errorText = "Invalid binary literal";
                    if (errorLine) *errorLine = tkLine;
                    return out;
                }
                out.append(makeNumber(v, tkLine, tkCol));
                continue;
            }
            advance();
            Token tk; tk.t = Tk::Percent; tk.line = tkLine; tk.col = tkCol;
            out.append(tk);
            continue;
        }

        if (c.isDigit())
        {
            // Scan digits possibly followed by H or B to detect suffixed literals.
            int q = p;
            bool sawAF = false;
            while (q < s.size())
            {
                QChar cc = s[q];
                if (cc.isDigit()) { q++; continue; }
                if ((cc >= 'A' && cc <= 'F') || (cc >= 'a' && cc <= 'f')) { q++; sawAF = true; continue; }
                break;
            }
            QChar suffix = q < s.size() ? s[q] : QChar(0);
            bool isHexSuffix = (suffix == 'H' || suffix == 'h');
            bool isBinSuffix = (suffix == 'B' || suffix == 'b') && !sawAF;

            if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X'))
            {
                advance(2);
                i64 v = 0;
                if (!readHex(v))
                {
                    if (errorText) *errorText = "Invalid hex literal";
                    if (errorLine) *errorLine = tkLine;
                    return out;
                }
                out.append(makeNumber(v, tkLine, tkCol));
                continue;
            }
            if (peek() == '0' && (peek(1) == 'b' || peek(1) == 'B'))
            {
                advance(2);
                i64 v = 0;
                if (!readBin(v))
                {
                    if (errorText) *errorText = "Invalid binary literal";
                    if (errorLine) *errorLine = tkLine;
                    return out;
                }
                out.append(makeNumber(v, tkLine, tkCol));
                continue;
            }

            if (isHexSuffix)
            {
                i64 v = 0;
                readHex(v);
                advance();
                out.append(makeNumber(v, tkLine, tkCol));
                continue;
            }
            if (isBinSuffix)
            {
                i64 v = 0;
                readBin(v);
                advance();
                out.append(makeNumber(v, tkLine, tkCol));
                continue;
            }
            i64 v = 0;
            if (!readDec(v))
            {
                if (errorText) *errorText = "Invalid number";
                if (errorLine) *errorLine = tkLine;
                return out;
            }
            out.append(makeNumber(v, tkLine, tkCol));
            continue;
        }

        if (c == '"')
        {
            advance();
            QString val;
            while (!atEnd() && peek() != '"')
            {
                QChar cc = peek();
                if (cc == '\\' && p + 1 < s.size())
                {
                    QChar esc = peek(1);
                    advance(2);
                    switch (esc.unicode())
                    {
                    case 'n': val += '\n'; break;
                    case 't': val += '\t'; break;
                    case 'r': val += '\r'; break;
                    case '0': val += QChar(0); break;
                    case '\\': val += '\\'; break;
                    case '"': val += '"'; break;
                    default: val += esc; break;
                    }
                    continue;
                }
                if (cc == '\n') break;
                val += cc;
                advance();
            }
            if (peek() != '"')
            {
                if (errorText) *errorText = "Unterminated string";
                if (errorLine) *errorLine = tkLine;
                return out;
            }
            advance();
            Token tk; tk.t = Tk::String; tk.text = val; tk.line = tkLine; tk.col = tkCol;
            out.append(tk);
            continue;
        }

        if (c == '\'')
        {
            if (!out.isEmpty() && out.last().t == Tk::Ident && out.last().text == "af")
            {
                advance();
                Token tk; tk.t = Tk::Apos; tk.line = tkLine; tk.col = tkCol;
                out.append(tk);
                continue;
            }
            advance();
            QString val;
            while (!atEnd() && peek() != '\'')
            {
                QChar cc = peek();
                if (cc == '\\' && p + 1 < s.size())
                {
                    QChar esc = peek(1);
                    advance(2);
                    switch (esc.unicode())
                    {
                    case 'n': val += '\n'; break;
                    case 't': val += '\t'; break;
                    case 'r': val += '\r'; break;
                    case '0': val += QChar(0); break;
                    case '\\': val += '\\'; break;
                    case '\'': val += '\''; break;
                    default: val += esc; break;
                    }
                    continue;
                }
                if (cc == '\n') break;
                val += cc;
                advance();
            }
            if (peek() != '\'')
            {
                if (errorText) *errorText = "Unterminated character literal";
                if (errorLine) *errorLine = tkLine;
                return out;
            }
            advance();
            Token tk;
            if (val.size() == 1)
            {
                tk.t = Tk::Number;
                tk.n = val[0].unicode();
            }
            else
            {
                tk.t = Tk::String;
                tk.text = val;
            }
            tk.line = tkLine; tk.col = tkCol;
            out.append(tk);
            continue;
        }

        if (isIdStart(c))
        {
            QString name;
            name += peek();
            advance();
            while (!atEnd() && isIdCont(peek()))
            {
                name += peek();
                advance();
            }
            Token tk;
            tk.t = Tk::Ident;
            tk.text = name.toLower();
            tk.line = tkLine;
            tk.col = tkCol;
            out.append(tk);
            continue;
        }

        auto tk1 = [&](Tk t) { advance(); Token x; x.t = t; x.line = tkLine; x.col = tkCol; return x; };
        auto tk2 = [&](Tk t) { advance(2); Token x; x.t = t; x.line = tkLine; x.col = tkCol; return x; };

        QChar n2 = peek(1);
        switch (c.unicode())
        {
        case ',': out.append(tk1(Tk::Comma)); continue;
        case ':': out.append(tk1(Tk::Colon)); continue;
        case '(': out.append(tk1(Tk::LP)); continue;
        case ')': out.append(tk1(Tk::RP)); continue;
        case '+': out.append(tk1(Tk::Plus)); continue;
        case '-': out.append(tk1(Tk::Minus)); continue;
        case '*': out.append(tk1(Tk::Star)); continue;
        case '/': out.append(tk1(Tk::Slash)); continue;
        case '~': out.append(tk1(Tk::Tilde)); continue;
        case '^': out.append(tk1(Tk::Caret)); continue;
        case '|':
            if (n2 == '|') out.append(tk2(Tk::LogOr));
            else out.append(tk1(Tk::Pipe));
            continue;
        case '<':
            if (n2 == '<') out.append(tk2(Tk::Shl));
            else if (n2 == '=') out.append(tk2(Tk::LtEq));
            else if (n2 == '>') out.append(tk2(Tk::NotEq));
            else out.append(tk1(Tk::Lt));
            continue;
        case '>':
            if (n2 == '>') out.append(tk2(Tk::Shr));
            else if (n2 == '=') out.append(tk2(Tk::GtEq));
            else out.append(tk1(Tk::Gt));
            continue;
        case '=':
            if (n2 == '=') out.append(tk2(Tk::Eq));
            else out.append(tk1(Tk::Eq));
            continue;
        case '!':
            if (n2 == '=') out.append(tk2(Tk::NotEq));
            else out.append(tk1(Tk::Bang));
            continue;
        default:
            if (errorText) *errorText = QString("Unexpected character '%1'").arg(c);
            if (errorLine) *errorLine = tkLine;
            return out;
        }
    }

    Token end; end.t = Tk::End; end.line = line; end.col = col;
    out.append(end);
    if (!src.isEmpty())
        for (Token &t : out) t.source = src;
    return out;
}

// ============================================================
// Core assembler
// ============================================================

struct Core
{
    AssemblerResult run(const QString &source, const QString &basePath,
                        const Assembler::ProgressFn &progress);

    QVector<Token> toks;
    int ti = 0;

    QHash<QString, i64> symbols;

    QVector<AssemblerSegment> segments;

    int pc = 0;
    int writePc = 0;
    int pass = 1;
    int curLowerRom = -1;
    int curUpperRom = -1;
    int curRamBank  = 0xC0;
    QString curFile;
    bool curToDisk = false;
    int curExec = -1;
    QString basePath;
    int includeDepth = 0;
    int estimatedTokens = 0;
    bool codeEnabled = true;
    int codeLimit = -1;
    bool limitErrorReported = false;
    bool endHit = false;
    bool hadError = false;
    QVector<AssemblerMessage> messages;
    Assembler::ProgressFn progressFn;

    struct CondFrame { bool parentActive; bool active; bool takenThisIf; };
    QVector<CondFrame> condStack;

    bool condActive() const
    {
        for (const CondFrame &f : condStack) if (!f.active) return false;
        return true;
    }

    void handleConditional(const QString &id);

    struct Macro { QString name; QVector<QString> params; QVector<Token> body; };
    QHash<QString, Macro> macros;
    int totalMacroExpansions = 0;

    void handleMacroDefinition();
    void handleMacroInvocation(const Macro &m);

    const Token &cur() const { return toks[ti]; }
    const Token &peekTok(int o = 0) const { return toks[ti + o]; }
    void skipEols() { while (cur().t == Tk::Eol || cur().t == Tk::Colon) ti++; }

    void error(int line, const QString &text)
    {
        AssemblerMessage m; m.line = line; m.text = text; m.isError = true;
        m.source = cur().source;
        messages.append(m);
        hadError = true;
    }
    void errorHere(const QString &text) { error(cur().line, text); }
    void info(int line, const QString &text)
    {
        AssemblerMessage m; m.line = line; m.text = text; m.isError = false;
        m.source = cur().source;
        messages.append(m);
    }

    QString interpolatePrintString(const QString &s) const
    {
        QString out;
        for (int i = 0; i < s.size(); )
        {
            QChar c = s[i];
            if (c == '$' || c == '&')
            {
                int j = i + 1;
                if (j < s.size() && (s[j].isLetter() || s[j] == '_' || s[j] == '.' || s[j] == '@'))
                {
                    while (j < s.size() &&
                           (s[j].isLetterOrNumber() || s[j] == '_' || s[j] == '.' || s[j] == '?'))
                        j++;
                    QString name = s.mid(i + 1, j - i - 1).toLower();
                    auto it = symbols.find(name);
                    if (it != symbols.end())
                    {
                        if (c == '$')
                            out += QString::number(it.value() & 0xFFFF, 16).toUpper()
                                       .rightJustified(4, QLatin1Char('0'));
                        else
                            out += QString::number(it.value());
                        i = j;
                        continue;
                    }
                }
            }
            out += c;
            i++;
        }
        return out;
    }

    QString resolveIncludePath(const QString &name) const
    {
        QFileInfo fi(name);
        if (fi.isAbsolute()) return fi.absoluteFilePath();
        QString ctxDir;
        if (!cur().source.isEmpty())
            ctxDir = QFileInfo(cur().source).absolutePath();
        else
            ctxDir = basePath;
        if (!ctxDir.isEmpty())
        {
            QString candidate = QDir(ctxDir).absoluteFilePath(name);
            if (QFileInfo::exists(candidate)) return candidate;
        }
        if (!basePath.isEmpty() && basePath != ctxDir)
        {
            QString candidate = QDir(basePath).absoluteFilePath(name);
            if (QFileInfo::exists(candidate)) return candidate;
        }
        if (ctxDir.isEmpty()) return QFileInfo(name).absoluteFilePath();
        return QDir(ctxDir).absoluteFilePath(name);
    }

    void emitByte(int value)
    {
        bool overLimit = (codeLimit >= 0 && pc > codeLimit);
        if (overLimit)
        {
            if (!limitErrorReported)
            {
                error(cur().line,
                      QString("code exceeds LIMIT &%1 (pc=&%2)")
                          .arg(QString::number(codeLimit, 16).toUpper())
                          .arg(QString::number(pc, 16).toUpper()));
                limitErrorReported = true;
            }
        }
        else
        {
            limitErrorReported = false;
        }
        if (pass == 2 && codeEnabled && !overLimit)
        {
            if (segments.isEmpty())
            {
                AssemblerSegment s;
                s.origin = pc;
                s.writeOrigin = writePc;
                s.lowerRom = curLowerRom;
                s.upperRom = curUpperRom;
                s.ramBank  = curRamBank;
                s.fileName = curFile;
                s.toDisk = curToDisk;
                s.execAddress = curExec;
                segments.append(s);
            }
            segments.last().bytes.append(char(value & 0xFF));
        }
        pc = (pc + 1) & 0xFFFF;
        writePc = (writePc + 1) & 0xFFFF;
    }
    void emitWord(int value) { emitByte(value & 0xFF); emitByte((value >> 8) & 0xFF); }

    void startSegment()
    {
        if (pass != 2) return;
        AssemblerSegment s;
        s.origin = pc;
        s.writeOrigin = writePc;
        s.lowerRom = curLowerRom;
        s.upperRom = curUpperRom;
        s.ramBank  = curRamBank;
        s.fileName = curFile;
        s.toDisk = curToDisk;
        s.execAddress = curExec;
        segments.append(s);
    }

    void retargetSegment()
    {
        if (pass != 2) return;
        if (segments.isEmpty() || !segments.last().bytes.isEmpty())
        {
            AssemblerSegment s;
            s.origin = pc;
            s.writeOrigin = writePc;
            s.lowerRom = curLowerRom;
            s.upperRom = curUpperRom;
            s.ramBank  = curRamBank;
            s.fileName = curFile;
            s.toDisk = curToDisk;
            s.execAddress = curExec;
            segments.append(s);
        }
        else
        {
            segments.last().origin       = pc;
            segments.last().writeOrigin  = writePc;
            segments.last().lowerRom     = curLowerRom;
            segments.last().upperRom     = curUpperRom;
            segments.last().ramBank      = curRamBank;
            segments.last().fileName     = curFile;
            segments.last().toDisk       = curToDisk;
            segments.last().execAddress  = curExec;
        }
    }

    struct ExprResult { i64 value = 0; bool defined = true; };
    ExprResult parseExpr();
    ExprResult parseLogicOr();
    ExprResult parseLogicAnd();
    ExprResult parseBitOr();
    ExprResult parseBitXor();
    ExprResult parseBitAnd();
    ExprResult parseEquality();
    ExprResult parseComparison();
    ExprResult parseShift();
    ExprResult parseAdd();
    ExprResult parseMul();
    ExprResult parseUnary();
    ExprResult parsePrimary();

    void skipToEndOfStatement();

    void parseLine();
    bool parseDirective(const QString &name);
    void parseInstruction(const QString &mnemonic);

    struct Operand
    {
        enum K {
            None,
            Reg8, Reg16, Flag,
            Immediate,
            IndReg16,
            IndIX,
            IndIY,
            IndC,
            IndImm,
            String,
        } kind = None;

        int r8 = 0;
        int r16 = 0;
        int flag = 0;

        i64 value = 0;
        bool valueDefined = true;

        i64 disp = 0;
        bool hasDisp = false;

        QString strText;

        int line = 0;
    };

    Operand parseOperand();

    void encLD(const QVector<Operand> &ops, int ln);
    void encPUSHPOP(bool isPush, const QVector<Operand> &ops, int ln);
    void encJP(const QVector<Operand> &ops, int ln);
    void encJR(const QVector<Operand> &ops, int ln);
    void encCALL(const QVector<Operand> &ops, int ln);
    void encRET(const QVector<Operand> &ops, int ln);
    void encRST(const QVector<Operand> &ops, int ln);
    void encINCDEC(bool isInc, const QVector<Operand> &ops, int ln);
    void encADD(const QVector<Operand> &ops, int ln);
    void encADC(const QVector<Operand> &ops, int ln);
    void encSBC(const QVector<Operand> &ops, int ln);
    void encALU(int op, const QVector<Operand> &ops, int ln);
    void encIN(const QVector<Operand> &ops, int ln);
    void encOUT(const QVector<Operand> &ops, int ln);
    void encEX(const QVector<Operand> &ops, int ln);
    void encIM(const QVector<Operand> &ops, int ln);
    void encCBRotateShift(int op, const QVector<Operand> &ops, int ln);
    void encBitOp(int baseCB, const QVector<Operand> &ops, int ln);
    void encDJNZ(const QVector<Operand> &ops, int ln);

    static int flagCode(const QString &n);
    static int reg8Code(const QString &n);
    static int reg16Code(const QString &n);
};

int Core::flagCode(const QString &n)
{
    if (n == "nz") return 0;
    if (n == "z")  return 1;
    if (n == "nc") return 2;
    if (n == "c")  return 3;
    if (n == "po") return 4;
    if (n == "pe") return 5;
    if (n == "p")  return 6;
    if (n == "m")  return 7;
    return -1;
}

int Core::reg8Code(const QString &n)
{
    if (n == "b") return 0;
    if (n == "c") return 1;
    if (n == "d") return 2;
    if (n == "e") return 3;
    if (n == "h") return 4;
    if (n == "l") return 5;
    if (n == "a") return 7;
    if (n == "i") return -1;
    if (n == "r") return -2;
    if (n == "ixh" || n == "xh" || n == "hx") return 100;
    if (n == "ixl" || n == "xl" || n == "lx") return 101;
    if (n == "iyh" || n == "yh" || n == "hy") return 102;
    if (n == "iyl" || n == "yl" || n == "ly") return 103;
    return -99;
}

int Core::reg16Code(const QString &n)
{
    if (n == "bc") return 0;
    if (n == "de") return 1;
    if (n == "hl") return 2;
    if (n == "sp") return 3;
    if (n == "af") return 4;
    if (n == "ix") return 6;
    if (n == "iy") return 7;
    return -1;
}

// ============================================================
// Expression parser
// ============================================================

Core::ExprResult Core::parseExpr() { return parseLogicOr(); }

Core::ExprResult Core::parseLogicOr()
{
    ExprResult l = parseLogicAnd();
    while (cur().t == Tk::LogOr)
    {
        ti++;
        ExprResult r = parseLogicAnd();
        l.value = (l.value != 0 || r.value != 0) ? 1 : 0;
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseLogicAnd()
{
    ExprResult l = parseBitOr();
    while (cur().t == Tk::LogAnd)
    {
        ti++;
        ExprResult r = parseBitOr();
        l.value = (l.value != 0 && r.value != 0) ? 1 : 0;
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseBitOr()
{
    ExprResult l = parseBitXor();
    while (cur().t == Tk::Pipe)
    {
        ti++;
        ExprResult r = parseBitXor();
        l.value |= r.value;
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseBitXor()
{
    ExprResult l = parseBitAnd();
    while (cur().t == Tk::Caret)
    {
        ti++;
        ExprResult r = parseBitAnd();
        l.value ^= r.value;
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseBitAnd()
{
    ExprResult l = parseEquality();
    while (cur().t == Tk::Amp)
    {
        ti++;
        ExprResult r = parseEquality();
        l.value &= r.value;
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseEquality()
{
    ExprResult l = parseComparison();
    while (cur().t == Tk::Eq || cur().t == Tk::NotEq)
    {
        Tk op = cur().t; ti++;
        ExprResult r = parseComparison();
        l.value = (op == Tk::Eq) ? (l.value == r.value ? 1 : 0) : (l.value != r.value ? 1 : 0);
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseComparison()
{
    ExprResult l = parseShift();
    while (cur().t == Tk::Lt || cur().t == Tk::Gt || cur().t == Tk::LtEq || cur().t == Tk::GtEq)
    {
        Tk op = cur().t; ti++;
        ExprResult r = parseShift();
        switch (op)
        {
        case Tk::Lt:   l.value = (l.value <  r.value) ? 1 : 0; break;
        case Tk::Gt:   l.value = (l.value >  r.value) ? 1 : 0; break;
        case Tk::LtEq: l.value = (l.value <= r.value) ? 1 : 0; break;
        case Tk::GtEq: l.value = (l.value >= r.value) ? 1 : 0; break;
        default: break;
        }
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseShift()
{
    ExprResult l = parseAdd();
    while (cur().t == Tk::Shl || cur().t == Tk::Shr)
    {
        Tk op = cur().t; ti++;
        ExprResult r = parseAdd();
        if (op == Tk::Shl) l.value <<= r.value;
        else               l.value >>= r.value;
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseAdd()
{
    ExprResult l = parseMul();
    while (cur().t == Tk::Plus || cur().t == Tk::Minus)
    {
        Tk op = cur().t; ti++;
        ExprResult r = parseMul();
        if (op == Tk::Plus) l.value += r.value;
        else                l.value -= r.value;
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseMul()
{
    ExprResult l = parseUnary();
    while (cur().t == Tk::Star || cur().t == Tk::Slash || cur().t == Tk::Percent)
    {
        Tk op = cur().t; ti++;
        ExprResult r = parseUnary();
        if (op == Tk::Star) l.value *= r.value;
        else if (op == Tk::Slash)
        {
            if (r.value == 0) { errorHere("Division by zero"); l.value = 0; }
            else l.value /= r.value;
        }
        else
        {
            if (r.value == 0) { errorHere("Modulo by zero"); l.value = 0; }
            else l.value %= r.value;
        }
        l.defined = l.defined && r.defined;
    }
    return l;
}

Core::ExprResult Core::parseUnary()
{
    if (cur().t == Tk::Plus)  { ti++; return parseUnary(); }
    if (cur().t == Tk::Minus) { ti++; ExprResult r = parseUnary(); r.value = -r.value; return r; }
    if (cur().t == Tk::Tilde) { ti++; ExprResult r = parseUnary(); r.value = ~r.value; return r; }
    if (cur().t == Tk::Bang)  { ti++; ExprResult r = parseUnary(); r.value = (r.value == 0) ? 1 : 0; return r; }
    return parsePrimary();
}

Core::ExprResult Core::parsePrimary()
{
    const Token &t = cur();
    if (t.t == Tk::Number) { ti++; ExprResult r; r.value = t.n; return r; }
    if (t.t == Tk::Dollar) { ti++; ExprResult r; r.value = pc; return r; }
    if (t.t == Tk::LP)
    {
        ti++;
        ExprResult r = parseExpr();
        if (cur().t != Tk::RP) errorHere("Expected ')'");
        else ti++;
        return r;
    }
    if (t.t == Tk::String)
    {
        ti++;
        ExprResult r;
        r.value = t.text.isEmpty() ? 0 : t.text[0].unicode();
        return r;
    }
    if (t.t == Tk::Ident)
    {
        ti++;
        if (t.text == "@") { ExprResult r; r.value = writePc; return r; }
        auto it = symbols.find(t.text);
        if (it == symbols.end())
        {
            ExprResult r;
            r.value = 0;
            r.defined = false;
            if (pass == 2)
                error(t.line, QString("Undefined symbol '%1'").arg(t.text));
            return r;
        }
        ExprResult r;
        r.value = it.value();
        return r;
    }
    errorHere("Expected expression");
    ExprResult r; r.value = 0; r.defined = false;
    return r;
}

void Core::skipToEndOfStatement()
{
    while (cur().t != Tk::Eol && cur().t != Tk::End && cur().t != Tk::Colon) ti++;
}

// ============================================================
// Operand parsing
// ============================================================

Core::Operand Core::parseOperand()
{
    Operand o;
    o.line = cur().line;

    if (cur().t == Tk::LP)
    {
        int save = ti;
        ti++;
        if (cur().t == Tk::Ident)
        {
            QString name = cur().text;
            int rc = reg16Code(name);
            if (rc >= 0 && rc <= 3 && peekTok(1).t == Tk::RP)
            {
                ti += 2;
                o.kind = Operand::IndReg16;
                o.r16 = rc;
                return o;
            }
            if (name == "c" && peekTok(1).t == Tk::RP)
            {
                ti += 2;
                o.kind = Operand::IndC;
                return o;
            }
            if (rc == 6 || rc == 7)
            {
                int idx = rc;
                if (peekTok(1).t == Tk::RP)
                {
                    ti += 2;
                    o.kind = (idx == 6) ? Operand::IndIX : Operand::IndIY;
                    o.hasDisp = false;
                    o.disp = 0;
                    return o;
                }
                if (peekTok(1).t == Tk::Plus || peekTok(1).t == Tk::Minus)
                {
                    ti++;
                    bool neg = (cur().t == Tk::Minus);
                    ti++;
                    ExprResult er = parseExpr();
                    if (cur().t != Tk::RP) errorHere("Expected ')'");
                    else ti++;
                    o.kind = (idx == 6) ? Operand::IndIX : Operand::IndIY;
                    o.hasDisp = true;
                    o.disp = neg ? -er.value : er.value;
                    return o;
                }
            }
        }
        ti = save;
        ti++;
        ExprResult er = parseExpr();
        if (cur().t != Tk::RP) errorHere("Expected ')'");
        else ti++;
        o.kind = Operand::IndImm;
        o.value = er.value;
        o.valueDefined = er.defined;
        return o;
    }

    if (cur().t == Tk::String)
    {
        if (cur().text.size() == 1)
        {
            o.kind = Operand::Immediate;
            o.value = cur().text[0].unicode();
            o.valueDefined = true;
            ti++;
            return o;
        }
        o.kind = Operand::String;
        o.strText = cur().text;
        ti++;
        return o;
    }

    if (cur().t == Tk::Ident)
    {
        QString name = cur().text;
        if (name == "af" && peekTok(1).t == Tk::Apos)
        {
            ti += 2;
            o.kind = Operand::Reg16;
            o.r16 = 5;
            return o;
        }
        int rc16 = reg16Code(name);
        if (rc16 >= 0)
        {
            Tk nxt = peekTok(1).t;
            if (nxt == Tk::Comma || nxt == Tk::Eol || nxt == Tk::End || nxt == Tk::RP || nxt == Tk::Colon)
            {
                ti++;
                o.kind = Operand::Reg16;
                o.r16 = rc16;
                return o;
            }
        }
        int rc8 = reg8Code(name);
        if (rc8 != -99)
        {
            Tk nxt = peekTok(1).t;
            bool isRegContext = (nxt == Tk::Comma || nxt == Tk::Eol || nxt == Tk::End || nxt == Tk::RP || nxt == Tk::Colon);
            if (isRegContext)
            {
                ti++;
                o.kind = Operand::Reg8;
                o.r8 = rc8;
                return o;
            }
        }
        int flag = flagCode(name);
        if (flag >= 0)
        {
            Tk nxt = peekTok(1).t;
            if (nxt == Tk::Comma || nxt == Tk::Eol || nxt == Tk::End ||
                nxt == Tk::Colon || nxt == Tk::RP)
            {
                ti++;
                o.kind = Operand::Flag;
                o.flag = flag;
                return o;
            }
        }
    }

    ExprResult er = parseExpr();
    o.kind = Operand::Immediate;
    o.value = er.value;
    o.valueDefined = er.defined;
    return o;
}

// ============================================================
// Encoding helpers
// ============================================================

static bool reg8WithPrefix(const Core::Operand &o, int &prefix, int &code)
{
    prefix = 0;
    if (o.kind != Core::Operand::Reg8) return false;
    if (o.r8 >= 0 && o.r8 <= 7) { code = o.r8; return true; }
    switch (o.r8)
    {
    case 100: prefix = 0xDD; code = 4; return true;
    case 101: prefix = 0xDD; code = 5; return true;
    case 102: prefix = 0xFD; code = 4; return true;
    case 103: prefix = 0xFD; code = 5; return true;
    default:  return false;
    }
}

static bool isIndexIndirect(const Core::Operand &o, int &prefix)
{
    if (o.kind == Core::Operand::IndIX) { prefix = 0xDD; return true; }
    if (o.kind == Core::Operand::IndIY) { prefix = 0xFD; return true; }
    return false;
}

// ============================================================
// Instruction encoders
// ============================================================

void Core::encLD(const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 2) { error(ln, "LD requires 2 operands"); return; }
    const Operand &d = ops[0];
    const Operand &s = ops[1];

    if (d.kind == Operand::Reg8 && d.r8 == 7 && s.kind == Operand::Reg8 && s.r8 == -1) { emitByte(0xED); emitByte(0x57); return; }
    if (d.kind == Operand::Reg8 && d.r8 == 7 && s.kind == Operand::Reg8 && s.r8 == -2) { emitByte(0xED); emitByte(0x5F); return; }
    if (d.kind == Operand::Reg8 && d.r8 == -1 && s.kind == Operand::Reg8 && s.r8 == 7) { emitByte(0xED); emitByte(0x47); return; }
    if (d.kind == Operand::Reg8 && d.r8 == -2 && s.kind == Operand::Reg8 && s.r8 == 7) { emitByte(0xED); emitByte(0x4F); return; }

    if (d.kind == Operand::Reg8 && s.kind == Operand::Reg8)
    {
        int dp, dc, sp, sc;
        if (!reg8WithPrefix(d, dp, dc) || !reg8WithPrefix(s, sp, sc))
        { error(ln, "Invalid LD operands"); return; }
        if (dp != 0 && sp != 0 && dp != sp) { error(ln, "Cannot mix IX and IY halves"); return; }
        int prefix = dp ? dp : sp;
        if (prefix) emitByte(prefix);
        emitByte(0x40 | (dc << 3) | sc);
        return;
    }

    if (d.kind == Operand::Reg8 && s.kind == Operand::Immediate)
    {
        int dp, dc;
        if (!reg8WithPrefix(d, dp, dc)) { error(ln, "Invalid LD destination"); return; }
        if (pass == 2 && (s.value < -128 || s.value > 255))
            error(ln, QString("8-bit immediate out of range: %1").arg(s.value));
        if (dp) emitByte(dp);
        emitByte(0x06 | (dc << 3));
        emitByte(int(s.value) & 0xFF);
        return;
    }

    if (d.kind == Operand::Reg8 && s.kind == Operand::IndReg16 && s.r16 == 2)
    {
        int dp, dc;
        if (!reg8WithPrefix(d, dp, dc)) { error(ln, "Invalid LD"); return; }
        if (dp) { error(ln, "Cannot use IX/IY half with (HL)"); return; }
        emitByte(0x46 | (dc << 3));
        return;
    }
    if (d.kind == Operand::IndReg16 && d.r16 == 2 && s.kind == Operand::Reg8)
    {
        int sp, sc;
        if (!reg8WithPrefix(s, sp, sc)) { error(ln, "Invalid LD"); return; }
        if (sp) { error(ln, "Cannot use IX/IY half with (HL)"); return; }
        emitByte(0x70 | sc);
        return;
    }

    if (d.kind == Operand::IndReg16 && d.r16 == 2 && s.kind == Operand::Immediate)
    {
        emitByte(0x36);
        emitByte(int(s.value) & 0xFF);
        return;
    }

    if (d.kind == Operand::Reg8 && d.r8 == 7 && s.kind == Operand::IndReg16)
    {
        if (s.r16 == 0) { emitByte(0x0A); return; }
        if (s.r16 == 1) { emitByte(0x1A); return; }
    }
    if (d.kind == Operand::IndReg16 && s.kind == Operand::Reg8 && s.r8 == 7)
    {
        if (d.r16 == 0) { emitByte(0x02); return; }
        if (d.r16 == 1) { emitByte(0x12); return; }
    }

    if (d.kind == Operand::Reg8 && d.r8 == 7 && s.kind == Operand::IndImm) { emitByte(0x3A); emitWord(int(s.value) & 0xFFFF); return; }
    if (d.kind == Operand::IndImm && s.kind == Operand::Reg8 && s.r8 == 7) { emitByte(0x32); emitWord(int(d.value) & 0xFFFF); return; }

    int prefix = 0;
    if (d.kind == Operand::Reg8 && isIndexIndirect(s, prefix))
    {
        int dp, dc;
        if (!reg8WithPrefix(d, dp, dc)) { error(ln, "Invalid LD"); return; }
        if (dp) { error(ln, "Cannot use IX/IY half with (IX+d)/(IY+d)"); return; }
        if (dc == 6) { error(ln, "Invalid LD"); return; }
        if (pass == 2 && (s.disp < -128 || s.disp > 127)) error(ln, "Displacement out of range");
        emitByte(prefix);
        emitByte(0x46 | (dc << 3));
        emitByte(int(s.disp) & 0xFF);
        return;
    }
    if (isIndexIndirect(d, prefix) && s.kind == Operand::Reg8)
    {
        int sp, sc;
        if (!reg8WithPrefix(s, sp, sc)) { error(ln, "Invalid LD"); return; }
        if (sp) { error(ln, "Cannot use IX/IY half with (IX+d)/(IY+d)"); return; }
        if (sc == 6) { error(ln, "Invalid LD"); return; }
        if (pass == 2 && (d.disp < -128 || d.disp > 127)) error(ln, "Displacement out of range");
        emitByte(prefix);
        emitByte(0x70 | sc);
        emitByte(int(d.disp) & 0xFF);
        return;
    }
    if (isIndexIndirect(d, prefix) && s.kind == Operand::Immediate)
    {
        if (pass == 2 && (d.disp < -128 || d.disp > 127)) error(ln, "Displacement out of range");
        emitByte(prefix);
        emitByte(0x36);
        emitByte(int(d.disp) & 0xFF);
        emitByte(int(s.value) & 0xFF);
        return;
    }

    if (d.kind == Operand::Reg16 && s.kind == Operand::Immediate)
    {
        int r = d.r16;
        if (r == 6) { emitByte(0xDD); emitByte(0x21); emitWord(int(s.value) & 0xFFFF); return; }
        if (r == 7) { emitByte(0xFD); emitByte(0x21); emitWord(int(s.value) & 0xFFFF); return; }
        if (r >= 0 && r <= 3) { emitByte(0x01 | (r << 4)); emitWord(int(s.value) & 0xFFFF); return; }
        error(ln, "Invalid LD destination"); return;
    }

    if (d.kind == Operand::Reg16 && s.kind == Operand::IndImm)
    {
        int r = d.r16;
        int addr = int(s.value) & 0xFFFF;
        if (r == 2)      { emitByte(0x2A); emitWord(addr); return; }
        if (r == 6)      { emitByte(0xDD); emitByte(0x2A); emitWord(addr); return; }
        if (r == 7)      { emitByte(0xFD); emitByte(0x2A); emitWord(addr); return; }
        if (r == 0 || r == 1 || r == 3)
        { emitByte(0xED); emitByte(0x4B | (r << 4)); emitWord(addr); return; }
        error(ln, "Invalid LD destination"); return;
    }

    if (d.kind == Operand::IndImm && s.kind == Operand::Reg16)
    {
        int r = s.r16;
        int addr = int(d.value) & 0xFFFF;
        if (r == 2)      { emitByte(0x22); emitWord(addr); return; }
        if (r == 6)      { emitByte(0xDD); emitByte(0x22); emitWord(addr); return; }
        if (r == 7)      { emitByte(0xFD); emitByte(0x22); emitWord(addr); return; }
        if (r == 0 || r == 1 || r == 3)
        { emitByte(0xED); emitByte(0x43 | (r << 4)); emitWord(addr); return; }
        error(ln, "Invalid LD source"); return;
    }

    if (d.kind == Operand::Reg16 && d.r16 == 3 && s.kind == Operand::Reg16)
    {
        if (s.r16 == 2) { emitByte(0xF9); return; }
        if (s.r16 == 6) { emitByte(0xDD); emitByte(0xF9); return; }
        if (s.r16 == 7) { emitByte(0xFD); emitByte(0xF9); return; }
    }

    error(ln, "Invalid LD operands");
}

void Core::encPUSHPOP(bool isPush, const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 1 || ops[0].kind != Operand::Reg16)
    { error(ln, "PUSH/POP requires a 16-bit register"); return; }
    int r = ops[0].r16;
    if (r == 6) { emitByte(0xDD); emitByte(isPush ? 0xE5 : 0xE1); return; }
    if (r == 7) { emitByte(0xFD); emitByte(isPush ? 0xE5 : 0xE1); return; }
    int code;
    if (r == 0) code = 0;
    else if (r == 1) code = 1;
    else if (r == 2) code = 2;
    else if (r == 4) code = 3;
    else { error(ln, "PUSH/POP: invalid register"); return; }
    emitByte((isPush ? 0xC5 : 0xC1) | (code << 4));
}

void Core::encJP(const QVector<Operand> &ops, int ln)
{
    auto asFlag = [](const Operand &o, int &f) -> bool {
        if (o.kind == Operand::Flag) { f = o.flag; return true; }
        if (o.kind == Operand::Reg8 && o.r8 == 1) { f = 3; return true; }
        return false;
    };
    if (ops.size() == 1)
    {
        const Operand &o = ops[0];
        if (o.kind == Operand::IndReg16 && o.r16 == 2) { emitByte(0xE9); return; }
        if (o.kind == Operand::IndIX) { emitByte(0xDD); emitByte(0xE9); return; }
        if (o.kind == Operand::IndIY) { emitByte(0xFD); emitByte(0xE9); return; }
        if (o.kind == Operand::Immediate || o.kind == Operand::IndImm)
        {
            emitByte(0xC3);
            emitWord(int(o.value) & 0xFFFF);
            return;
        }
        error(ln, "Invalid JP operand"); return;
    }
    int f;
    if (ops.size() == 2 && asFlag(ops[0], f) &&
        (ops[1].kind == Operand::Immediate || ops[1].kind == Operand::IndImm))
    {
        emitByte(0xC2 | (f << 3));
        emitWord(int(ops[1].value) & 0xFFFF);
        return;
    }
    error(ln, "Invalid JP operands");
}

void Core::encJR(const QVector<Operand> &ops, int ln)
{
    int flag = -1;
    const Operand *target = nullptr;
    if (ops.size() == 1) { target = &ops[0]; }
    else if (ops.size() == 2 && ops[0].kind == Operand::Flag)
    {
        flag = ops[0].flag;
        if (flag > 3) { error(ln, "JR only supports NZ/Z/NC/C"); return; }
        target = &ops[1];
    }
    else if (ops.size() == 2 && ops[0].kind == Operand::Reg8 && ops[0].r8 == 1)
    {
        flag = 3;
        target = &ops[1];
    }
    else { error(ln, "Invalid JR operands"); return; }

    if (target->kind != Operand::Immediate) { error(ln, "JR requires an address"); return; }
    int addr = int(target->value) & 0xFFFF;
    int next = (pc + 2) & 0xFFFF;
    int disp = addr - next;
    if (pass == 2 && (disp < -128 || disp > 127))
        error(ln, QString("JR out of range (displacement %1)").arg(disp));
    emitByte(flag < 0 ? 0x18 : (0x20 | (flag << 3)));
    emitByte(disp & 0xFF);
}

void Core::encCALL(const QVector<Operand> &ops, int ln)
{
    auto asFlag = [](const Operand &o, int &f) -> bool {
        if (o.kind == Operand::Flag) { f = o.flag; return true; }
        if (o.kind == Operand::Reg8 && o.r8 == 1) { f = 3; return true; }
        return false;
    };
    if (ops.size() == 1 && (ops[0].kind == Operand::Immediate || ops[0].kind == Operand::IndImm))
    {
        emitByte(0xCD);
        emitWord(int(ops[0].value) & 0xFFFF);
        return;
    }
    int f;
    if (ops.size() == 2 && asFlag(ops[0], f) &&
        (ops[1].kind == Operand::Immediate || ops[1].kind == Operand::IndImm))
    {
        emitByte(0xC4 | (f << 3));
        emitWord(int(ops[1].value) & 0xFFFF);
        return;
    }
    error(ln, "Invalid CALL operands");
}

void Core::encRET(const QVector<Operand> &ops, int ln)
{
    if (ops.isEmpty()) { emitByte(0xC9); return; }
    if (ops.size() == 1 && ops[0].kind == Operand::Flag)
    { emitByte(0xC0 | (ops[0].flag << 3)); return; }
    if (ops.size() == 1 && ops[0].kind == Operand::Reg8 && ops[0].r8 == 1)
    { emitByte(0xC0 | (3 << 3)); return; }
    error(ln, "Invalid RET operands");
}

void Core::encRST(const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 1 || ops[0].kind != Operand::Immediate) { error(ln, "RST requires a constant"); return; }
    int v = int(ops[0].value);
    if (v < 0 || v > 0x38 || (v & 0x07)) { error(ln, "RST target must be 0/8/16/24/32/40/48/56"); return; }
    emitByte(0xC7 | v);
}

void Core::encINCDEC(bool isInc, const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 1) { error(ln, "INC/DEC requires 1 operand"); return; }
    const Operand &o = ops[0];

    if (o.kind == Operand::Reg8)
    {
        int dp, dc;
        if (!reg8WithPrefix(o, dp, dc)) { error(ln, "Invalid INC/DEC operand"); return; }
        if (dp) emitByte(dp);
        emitByte((isInc ? 0x04 : 0x05) | (dc << 3));
        return;
    }
    if (o.kind == Operand::Reg16)
    {
        int r = o.r16;
        if (r == 6) { emitByte(0xDD); emitByte(isInc ? 0x23 : 0x2B); return; }
        if (r == 7) { emitByte(0xFD); emitByte(isInc ? 0x23 : 0x2B); return; }
        if (r >= 0 && r <= 3) { emitByte((isInc ? 0x03 : 0x0B) | (r << 4)); return; }
        error(ln, "INC/DEC: invalid register"); return;
    }
    if (o.kind == Operand::IndReg16 && o.r16 == 2)
    {
        emitByte(isInc ? 0x34 : 0x35);
        return;
    }
    int prefix = 0;
    if (isIndexIndirect(o, prefix))
    {
        if (pass == 2 && (o.disp < -128 || o.disp > 127)) error(ln, "Displacement out of range");
        emitByte(prefix);
        emitByte(isInc ? 0x34 : 0x35);
        emitByte(int(o.disp) & 0xFF);
        return;
    }
    error(ln, "Invalid INC/DEC operand");
}

void Core::encALU(int op, const QVector<Operand> &ops, int ln)
{
    const Operand *src = nullptr;
    if (ops.size() == 1) src = &ops[0];
    else if (ops.size() == 2 && ops[0].kind == Operand::Reg8 && ops[0].r8 == 7) src = &ops[1];
    else { error(ln, "Invalid ALU operands"); return; }

    if (src->kind == Operand::Reg8)
    {
        int dp, dc;
        if (!reg8WithPrefix(*src, dp, dc)) { error(ln, "Invalid ALU operand"); return; }
        if (dp) emitByte(dp);
        emitByte(0x80 | (op << 3) | dc);
        return;
    }
    if (src->kind == Operand::IndReg16 && src->r16 == 2)
    { emitByte(0x80 | (op << 3) | 6); return; }
    int prefix = 0;
    if (isIndexIndirect(*src, prefix))
    {
        if (pass == 2 && (src->disp < -128 || src->disp > 127)) error(ln, "Displacement out of range");
        emitByte(prefix);
        emitByte(0x80 | (op << 3) | 6);
        emitByte(int(src->disp) & 0xFF);
        return;
    }
    if (src->kind == Operand::Immediate)
    {
        static const int immOpcodes[8] = {0xC6, 0xCE, 0xD6, 0xDE, 0xE6, 0xEE, 0xF6, 0xFE};
        emitByte(immOpcodes[op]);
        emitByte(int(src->value) & 0xFF);
        return;
    }
    error(ln, "Invalid ALU operand");
}

void Core::encADD(const QVector<Operand> &ops, int ln)
{
    if (ops.size() == 2 && ops[0].kind == Operand::Reg16)
    {
        int d = ops[0].r16;
        if (d == 2)
        {
            if (ops[1].kind != Operand::Reg16) { error(ln, "ADD HL, rr requires 16-bit source"); return; }
            int s = ops[1].r16;
            int code;
            if (s == 0) code = 0;
            else if (s == 1) code = 1;
            else if (s == 2) code = 2;
            else if (s == 3) code = 3;
            else { error(ln, "ADD HL: invalid source"); return; }
            emitByte(0x09 | (code << 4));
            return;
        }
        if (d == 6 || d == 7)
        {
            if (ops[1].kind != Operand::Reg16) { error(ln, "ADD IX/IY: invalid source"); return; }
            int s = ops[1].r16;
            int code;
            if (s == 0) code = 0;
            else if (s == 1) code = 1;
            else if (s == d) code = 2;
            else if (s == 3) code = 3;
            else { error(ln, "ADD IX/IY: invalid source"); return; }
            emitByte(d == 6 ? 0xDD : 0xFD);
            emitByte(0x09 | (code << 4));
            return;
        }
    }
    encALU(0, ops, ln);
}

void Core::encADC(const QVector<Operand> &ops, int ln)
{
    if (ops.size() == 2 && ops[0].kind == Operand::Reg16 && ops[0].r16 == 2 && ops[1].kind == Operand::Reg16)
    {
        int s = ops[1].r16;
        int code;
        if (s == 0) code = 0;
        else if (s == 1) code = 1;
        else if (s == 2) code = 2;
        else if (s == 3) code = 3;
        else { error(ln, "ADC HL: invalid source"); return; }
        emitByte(0xED); emitByte(0x4A | (code << 4));
        return;
    }
    encALU(1, ops, ln);
}

void Core::encSBC(const QVector<Operand> &ops, int ln)
{
    if (ops.size() == 2 && ops[0].kind == Operand::Reg16 && ops[0].r16 == 2 && ops[1].kind == Operand::Reg16)
    {
        int s = ops[1].r16;
        int code;
        if (s == 0) code = 0;
        else if (s == 1) code = 1;
        else if (s == 2) code = 2;
        else if (s == 3) code = 3;
        else { error(ln, "SBC HL: invalid source"); return; }
        emitByte(0xED); emitByte(0x42 | (code << 4));
        return;
    }
    encALU(3, ops, ln);
}

void Core::encIN(const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 2) { error(ln, "IN requires 2 operands"); return; }
    const Operand &d = ops[0];
    const Operand &s = ops[1];
    if (d.kind == Operand::Reg8 && d.r8 == 7 && s.kind == Operand::IndImm)
    { emitByte(0xDB); emitByte(int(s.value) & 0xFF); return; }
    if (d.kind == Operand::Reg8 && s.kind == Operand::IndC)
    {
        int code;
        if (d.r8 >= 0 && d.r8 <= 7) code = d.r8;
        else { error(ln, "Invalid IN destination"); return; }
        emitByte(0xED); emitByte(0x40 | (code << 3));
        return;
    }
    error(ln, "Invalid IN operands");
}

void Core::encOUT(const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 2) { error(ln, "OUT requires 2 operands"); return; }
    const Operand &d = ops[0];
    const Operand &s = ops[1];
    if (d.kind == Operand::IndImm && s.kind == Operand::Reg8 && s.r8 == 7)
    { emitByte(0xD3); emitByte(int(d.value) & 0xFF); return; }
    if (d.kind == Operand::IndC && s.kind == Operand::Reg8)
    {
        int code;
        if (s.r8 >= 0 && s.r8 <= 7) code = s.r8;
        else { error(ln, "Invalid OUT source"); return; }
        emitByte(0xED); emitByte(0x41 | (code << 3));
        return;
    }
    error(ln, "Invalid OUT operands");
}

void Core::encEX(const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 2) { error(ln, "EX requires 2 operands"); return; }
    const Operand &a = ops[0];
    const Operand &b = ops[1];
    if (a.kind == Operand::Reg16 && a.r16 == 4 && b.kind == Operand::Reg16 && b.r16 == 5)
    { emitByte(0x08); return; }
    if (a.kind == Operand::Reg16 && a.r16 == 1 && b.kind == Operand::Reg16 && b.r16 == 2)
    { emitByte(0xEB); return; }
    if (a.kind == Operand::Reg16 && a.r16 == 2 && b.kind == Operand::Reg16 && b.r16 == 1)
    { emitByte(0xEB); return; }
    if (a.kind == Operand::IndReg16 && a.r16 == 3 && b.kind == Operand::Reg16)
    {
        if (b.r16 == 2) { emitByte(0xE3); return; }
        if (b.r16 == 6) { emitByte(0xDD); emitByte(0xE3); return; }
        if (b.r16 == 7) { emitByte(0xFD); emitByte(0xE3); return; }
    }
    error(ln, "Invalid EX operands");
}

void Core::encIM(const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 1 || ops[0].kind != Operand::Immediate) { error(ln, "IM requires 0/1/2"); return; }
    int v = int(ops[0].value);
    emitByte(0xED);
    if (v == 0) emitByte(0x46);
    else if (v == 1) emitByte(0x56);
    else if (v == 2) emitByte(0x5E);
    else error(ln, "IM must be 0, 1, or 2");
}

void Core::encCBRotateShift(int op, const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 1) { error(ln, "Rotate/shift requires 1 operand"); return; }
    const Operand &o = ops[0];
    int base = (op << 3);
    if (o.kind == Operand::Reg8)
    {
        int dp, dc;
        if (!reg8WithPrefix(o, dp, dc)) { error(ln, "Invalid operand"); return; }
        if (dp) emitByte(dp);
        emitByte(0xCB);
        emitByte(base | dc);
        return;
    }
    if (o.kind == Operand::IndReg16 && o.r16 == 2)
    { emitByte(0xCB); emitByte(base | 6); return; }
    int prefix = 0;
    if (isIndexIndirect(o, prefix))
    {
        if (pass == 2 && (o.disp < -128 || o.disp > 127)) error(ln, "Displacement out of range");
        emitByte(prefix);
        emitByte(0xCB);
        emitByte(int(o.disp) & 0xFF);
        emitByte(base | 6);
        return;
    }
    error(ln, "Invalid operand");
}

void Core::encBitOp(int baseCB, const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 2 || ops[0].kind != Operand::Immediate)
    { error(ln, "BIT/RES/SET requires bit number and operand"); return; }
    int bit = int(ops[0].value);
    if (bit < 0 || bit > 7) { error(ln, "Bit number must be 0..7"); return; }

    const Operand &o = ops[1];
    int enc = baseCB | (bit << 3);
    if (o.kind == Operand::Reg8)
    {
        int dp, dc;
        if (!reg8WithPrefix(o, dp, dc)) { error(ln, "Invalid operand"); return; }
        if (dp) emitByte(dp);
        emitByte(0xCB);
        emitByte(enc | dc);
        return;
    }
    if (o.kind == Operand::IndReg16 && o.r16 == 2)
    { emitByte(0xCB); emitByte(enc | 6); return; }
    int prefix = 0;
    if (isIndexIndirect(o, prefix))
    {
        if (pass == 2 && (o.disp < -128 || o.disp > 127)) error(ln, "Displacement out of range");
        emitByte(prefix);
        emitByte(0xCB);
        emitByte(int(o.disp) & 0xFF);
        emitByte(enc | 6);
        return;
    }
    error(ln, "Invalid operand");
}

void Core::encDJNZ(const QVector<Operand> &ops, int ln)
{
    if (ops.size() != 1 || ops[0].kind != Operand::Immediate) { error(ln, "DJNZ requires an address"); return; }
    int addr = int(ops[0].value) & 0xFFFF;
    int next = (pc + 2) & 0xFFFF;
    int disp = addr - next;
    if (pass == 2 && (disp < -128 || disp > 127))
        error(ln, QString("DJNZ out of range (displacement %1)").arg(disp));
    emitByte(0x10);
    emitByte(disp & 0xFF);
}

// ============================================================
// Instruction dispatch
// ============================================================

void Core::parseInstruction(const QString &m)
{
    QVector<Operand> ops;
    int ln = cur().line;
    if (cur().t != Tk::Eol && cur().t != Tk::End && cur().t != Tk::Colon)
    {
        ops.append(parseOperand());
        while (cur().t == Tk::Comma)
        {
            ti++;
            ops.append(parseOperand());
        }
    }

    struct Simple { const char *m; int b1; int b2; };
    static const Simple table[] = {
        {"nop",  0x00, -1}, {"halt", 0x76, -1}, {"di", 0xF3, -1}, {"ei", 0xFB, -1},
        {"rlca", 0x07, -1}, {"rrca", 0x0F, -1}, {"rla", 0x17, -1}, {"rra", 0x1F, -1},
        {"daa",  0x27, -1}, {"cpl",  0x2F, -1}, {"scf", 0x37, -1}, {"ccf", 0x3F, -1},
        {"exx",  0xD9, -1},
        {"neg",  0xED, 0x44}, {"reti", 0xED, 0x4D}, {"retn", 0xED, 0x45},
        {"rld",  0xED, 0x6F}, {"rrd",  0xED, 0x67},
        {"ldi",  0xED, 0xA0}, {"ldir", 0xED, 0xB0}, {"ldd", 0xED, 0xA8}, {"lddr", 0xED, 0xB8},
        {"cpi",  0xED, 0xA1}, {"cpir", 0xED, 0xB1}, {"cpd", 0xED, 0xA9}, {"cpdr", 0xED, 0xB9},
        {"ini",  0xED, 0xA2}, {"inir", 0xED, 0xB2}, {"ind", 0xED, 0xAA}, {"indr", 0xED, 0xBA},
        {"outi", 0xED, 0xA3}, {"otir", 0xED, 0xB3}, {"outd", 0xED, 0xAB}, {"otdr", 0xED, 0xBB},
    };
    for (const Simple &s : table)
    {
        if (m == QString::fromLatin1(s.m))
        {
            if (!ops.isEmpty()) { error(ln, QString("%1 takes no operands").arg(m.toUpper())); return; }
            if (s.b1 >= 0) emitByte(s.b1);
            if (s.b2 >= 0) emitByte(s.b2);
            return;
        }
    }

    if (m == "ld")     { encLD(ops, ln); return; }
    if (m == "push")   { encPUSHPOP(true, ops, ln); return; }
    if (m == "pop")    { encPUSHPOP(false, ops, ln); return; }
    if (m == "jp")     { encJP(ops, ln); return; }
    if (m == "jr")     { encJR(ops, ln); return; }
    if (m == "call")   { encCALL(ops, ln); return; }
    if (m == "ret")    { encRET(ops, ln); return; }
    if (m == "rst")    { encRST(ops, ln); return; }
    if (m == "inc")    { encINCDEC(true, ops, ln); return; }
    if (m == "dec")    { encINCDEC(false, ops, ln); return; }
    if (m == "add")    { encADD(ops, ln); return; }
    if (m == "adc")    { encADC(ops, ln); return; }
    if (m == "sub")    { encALU(2, ops, ln); return; }
    if (m == "sbc")    { encSBC(ops, ln); return; }
    if (m == "and")    { encALU(4, ops, ln); return; }
    if (m == "xor")    { encALU(5, ops, ln); return; }
    if (m == "or")     { encALU(6, ops, ln); return; }
    if (m == "cp")     { encALU(7, ops, ln); return; }
    if (m == "in")     { encIN(ops, ln); return; }
    if (m == "out")    { encOUT(ops, ln); return; }
    if (m == "ex")     { encEX(ops, ln); return; }
    if (m == "im")     { encIM(ops, ln); return; }
    if (m == "djnz")   { encDJNZ(ops, ln); return; }
    if (m == "rlc")    { encCBRotateShift(0, ops, ln); return; }
    if (m == "rrc")    { encCBRotateShift(1, ops, ln); return; }
    if (m == "rl")     { encCBRotateShift(2, ops, ln); return; }
    if (m == "rr")     { encCBRotateShift(3, ops, ln); return; }
    if (m == "sla")    { encCBRotateShift(4, ops, ln); return; }
    if (m == "sra")    { encCBRotateShift(5, ops, ln); return; }
    if (m == "sll")    { encCBRotateShift(6, ops, ln); return; }
    if (m == "srl")    { encCBRotateShift(7, ops, ln); return; }
    if (m == "bit")    { encBitOp(0x40, ops, ln); return; }
    if (m == "res")    { encBitOp(0x80, ops, ln); return; }
    if (m == "set")    { encBitOp(0xC0, ops, ln); return; }

    error(ln, QString("Unknown mnemonic '%1'").arg(m));
}

// ============================================================
// Directives
// ============================================================

bool Core::parseDirective(const QString &name)
{
    int ln = cur().line;
    (void)ln;

    if (name == "nolist" || name == "list")
    {
        skipToEndOfStatement();
        return true;
    }
    if (name == "limit")
    {
        if (cur().t != Tk::Eol && cur().t != Tk::End && cur().t != Tk::Colon)
        {
            ExprResult er = parseExpr();
            codeLimit = int(er.value) & 0xFFFF;
            limitErrorReported = false;
        }
        return true;
    }
    if (name == "brk")
    {
        emitByte(0xF7);
        return true;
    }
    if (name == "str")
    {
        do
        {
            if (cur().t == Tk::String)
            {
                QString s = cur().text;
                ti++;
                if (s.isEmpty()) continue;
                for (int i = 0; i < s.size() - 1; i++)
                    emitByte(s[i].unicode() & 0xFF);
                emitByte((s[s.size() - 1].unicode() & 0xFF) | 0x80);
            }
            else
            {
                ExprResult er = parseExpr();
                emitByte((int(er.value) & 0xFF) | 0x80);
            }
            if (cur().t != Tk::Comma) break;
            ti++;
        } while (true);
        return true;
    }
    if (name == "nocode")
    {
        codeEnabled = false;
        return true;
    }
    if (name == "code")
    {
        bool wasDisabled = !codeEnabled;
        codeEnabled = true;
        if (wasDisabled) retargetSegment();
        return true;
    }
    if (name == "close")
    {
        curFile.clear();
        curToDisk = false;
        curExec = -1;
        curLowerRom = -1;
        curUpperRom = -1;
        curRamBank  = 0xC0;
        if (pass == 2) retargetSegment();
        return true;
    }
    if (name == "end")
    {
        endHit = true;
        return true;
    }
    if (name == "print")
    {
        do
        {
            QString text;
            if (cur().t == Tk::String)
            {
                QString s = cur().text; ti++;
                text = interpolatePrintString(s);
            }
            else
            {
                ExprResult er = parseExpr();
                text = QString::number(er.value);
            }
            if (pass == 2) info(ln, text);
            if (cur().t != Tk::Comma) break;
            ti++;
        } while (true);
        return true;
    }

    if (name == "org")
    {
        ExprResult er = parseExpr();
        int addr = int(er.value) & 0xFFFF;
        int out = addr;
        if (cur().t == Tk::Comma)
        {
            ti++;
            ExprResult er2 = parseExpr();
            out = int(er2.value) & 0xFFFF;
        }
        pc = addr;
        writePc = out;
        if (pass == 2)
        {
            if (segments.isEmpty() || !segments.last().bytes.isEmpty())
                startSegment();
            else
            {
                segments.last().origin      = pc;
                segments.last().writeOrigin = writePc;
            }
        }
        return true;
    }
    if (name == "db" || name == "defb" || name == "dm" || name == "defm" ||
        name == "text" || name == "byte")
    {
        do
        {
            if (cur().t == Tk::String)
            {
                Tk nxt = peekTok(1).t;
                bool standalone = (nxt == Tk::Comma || nxt == Tk::Eol ||
                                   nxt == Tk::End || nxt == Tk::Colon);
                if (standalone)
                {
                    QString s = cur().text; ti++;
                    for (QChar c : s) emitByte(c.unicode() & 0xFF);
                }
                else
                {
                    ExprResult er = parseExpr();
                    emitByte(int(er.value) & 0xFF);
                }
            }
            else
            {
                ExprResult er = parseExpr();
                emitByte(int(er.value) & 0xFF);
            }
            if (cur().t != Tk::Comma) break;
            ti++;
        } while (true);
        return true;
    }
    if (name == "dw" || name == "defw" || name == "word")
    {
        do
        {
            ExprResult er = parseExpr();
            emitWord(int(er.value) & 0xFFFF);
            if (cur().t != Tk::Comma) break;
            ti++;
        } while (true);
        return true;
    }
    if (name == "ds" || name == "defs" || name == "rmem")
    {
        ExprResult count = parseExpr();
        if (!count.defined) { error(ln, "DS size must be known in pass 1"); skipToEndOfStatement(); return true; }
        i64 fill = 0;
        if (cur().t == Tk::Comma)
        {
            ti++;
            ExprResult er = parseExpr();
            fill = er.value & 0xFF;
        }
        if (count.value < 0 || count.value > 0x10000)
        {
            error(ln, QString("DS count out of range: %1 (pc=&%2)").arg(qint64(count.value)).arg(QString::number(pc, 16).toUpper()));
            return true;
        }
        for (i64 i = 0; i < count.value; i++) emitByte(int(fill) & 0xFF);
        return true;
    }
    if (name == "align")
    {
        ExprResult er = parseExpr();
        if (!er.defined) { error(ln, "ALIGN value must be known in pass 1"); skipToEndOfStatement(); return true; }
        int n = int(er.value);
        if (n <= 0) { error(ln, "ALIGN value must be positive"); return true; }
        while (pc % n != 0) emitByte(0x00);
        return true;
    }
    if (name == "assert")
    {
        ExprResult er = parseExpr();
        if (pass == 2 && er.value == 0)
            error(ln, "ASSERT failed");
        return true;
    }
    if (name == "read")
    {
        if (cur().t != Tk::String)
        {
            error(ln, "READ: expected \"filename\"");
            skipToEndOfStatement();
            return true;
        }
        QString fname = cur().text;
        ti++;
        if (pass == 1)
        {
            if (includeDepth >= 32)
            {
                error(ln, "READ: include depth exceeds 32 (circular READ?)");
                return true;
            }
            QString resolved = resolveIncludePath(fname);
            QFile f(resolved);
            if (!f.open(QFile::ReadOnly | QFile::Text))
            {
                error(ln, QString("READ: cannot open \"%1\"").arg(fname));
                return true;
            }
            QString text = QString::fromUtf8(f.readAll());
            Lexer lx(text, resolved);
            QString lxErr; int lxLn = 0;
            QVector<Token> incToks = lx.all(&lxErr, &lxLn);
            if (!lxErr.isEmpty())
            {
                error(ln, QString("READ \"%1\": %2 (line %3)").arg(fname, lxErr, QString::number(lxLn)));
                return true;
            }
            if (!incToks.isEmpty() && incToks.last().t == Tk::End)
                incToks.removeLast();
            Token eol; eol.t = Tk::Eol; eol.line = 1; eol.col = 1; eol.source = resolved;
            incToks.prepend(eol);
            QVector<Token> tail = toks.mid(ti);
            toks.resize(ti);
            toks.append(incToks);
            toks.append(tail);
            if (progressFn)
            {
                int total = estimatedTokens > 0 ? estimatedTokens
                           : (toks.size() ? toks.size() : 1);
                int pct = int((qint64(ti) * 100) / total);
                if (pct > 100) pct = 100;
                progressFn(1, pct, QString("READ %1").arg(QFileInfo(fname).fileName()));
            }
        }
        return true;
    }
    if (name == "incbin")
    {
        if (cur().t != Tk::String)
        {
            error(ln, "INCBIN: expected \"filename\"");
            skipToEndOfStatement();
            return true;
        }
        QString fname = cur().text;
        ti++;
        qint64 offset = 0;
        qint64 size = -1;
        qint64 offsetHigh = 0;
        bool haveSize = false;
        if (cur().t == Tk::Comma)
        {
            ti++;
            ExprResult er = parseExpr();
            offset = er.value & 0xFFFF;
            if (cur().t == Tk::Comma)
            {
                ti++;
                ExprResult er2 = parseExpr();
                size = er2.value & 0xFFFF;
                haveSize = true;
                if (cur().t == Tk::Comma)
                {
                    ti++;
                    ExprResult er3 = parseExpr();
                    offsetHigh = er3.value & 0xFFFF;
                }
            }
        }
        qint64 totalOffset = offsetHigh * 65536 + offset;
        QString resolved = resolveIncludePath(fname);
        QFile f(resolved);
        if (!f.open(QFile::ReadOnly))
        {
            if (pass == 1)
                error(ln, QString("INCBIN: cannot open \"%1\"").arg(fname));
            return true;
        }
        if (totalOffset > 0 && !f.seek(totalOffset))
        {
            if (pass == 1)
                error(ln, QString("INCBIN: cannot seek to offset %1 in \"%2\"")
                          .arg(totalOffset).arg(fname));
            return true;
        }
        QByteArray data = haveSize ? f.read(size) : f.readAll();
        for (char b : data) emitByte(static_cast<unsigned char>(b));
        return true;
    }
    if (name == "write")
    {
        if (cur().t == Tk::String)
        {
            QString fname = cur().text;
            ti++;
            if (cur().t == Tk::Comma)
            {
                if (pass == 2) error(ln, "WRITE \"file\" with start/length args not supported (use WRITE \"file\" only)");
                skipToEndOfStatement();
                return true;
            }
            if (pass == 2)
            {
                if (fname.isEmpty()) { error(ln, "WRITE: filename must not be empty"); return true; }
                curFile = fname;
                curLowerRom = -1;
                curUpperRom = -1;
                curRamBank  = 0xC0;
                curToDisk = false;
                curExec = -1;
                retargetSegment();
            }
            return true;
        }
        if (!(cur().t == Tk::Ident && cur().text == "direct"))
        {
            if (pass == 2) error(ln, "WRITE: expected \"filename\" or DIRECT");
            skipToEndOfStatement();
            return true;
        }
        ti++;
        if (cur().t == Tk::Ident && cur().text == "sectors")
        {
            if (pass == 2) error(ln, "WRITE DIRECT SECTORS not yet supported");
            skipToEndOfStatement();
            return true;
        }
        if (cur().t == Tk::String)
        {
            QString fname = cur().text;
            ti++;
            i64 execAddr = 0;
            bool haveExec = false;
            if (cur().t == Tk::Comma)
            {
                ti++;
                ExprResult er = parseExpr();
                execAddr = er.value & 0xFFFF;
                haveExec = true;
            }
            if (pass == 2)
            {
                if (fname.isEmpty()) { error(ln, "WRITE DIRECT: filename must not be empty"); return true; }
                curFile = fname;
                curLowerRom = -1;
                curUpperRom = -1;
                curRamBank  = 0xC0;
                curToDisk = true;
                curExec = haveExec ? int(execAddr) : -1;
                retargetSegment();
            }
            return true;
        }
        i64 lower = -1, upper = -1, bank = 0xC0;
        if (cur().t != Tk::Eol && cur().t != Tk::End && cur().t != Tk::Colon)
        {
            ExprResult er = parseExpr();
            lower = er.value;
            if (cur().t == Tk::Comma)
            {
                ti++;
                er = parseExpr();
                upper = er.value;
                if (cur().t == Tk::Comma)
                {
                    ti++;
                    er = parseExpr();
                    bank = er.value;
                }
            }
        }
        if (pass == 2)
        {
            if (lower != -1 && lower != 0)
            {
                error(ln, QString("WRITE: lower_rom must be -1 (disable) or 0 (enable), got %1").arg(qint64(lower)));
                return true;
            }
            if (upper != -1 && !(upper >= 0 && upper <= 15) && !(upper >= 0x80 && upper <= 0x9F))
            {
                error(ln, QString("WRITE: upper_rom must be -1, 0..15, or &80..&9F (cartridge block), got %1").arg(qint64(upper)));
                return true;
            }
            if (bank < 0xC0 || bank > 0xC7)
            {
                error(ln, QString("WRITE: ram_bank must be #C0..#C7 (Yarek 4MB banks not supported), got &%1")
                          .arg(QString::number(qint64(bank), 16).toUpper()));
                return true;
            }
            curLowerRom = int(lower);
            curUpperRom = int(upper);
            curRamBank  = int(bank);
            curFile.clear();
            curToDisk = false;
            curExec = -1;
            retargetSegment();
        }
        return true;
    }
    return false;
}

// ============================================================
// Line parsing
// ============================================================

void Core::handleMacroDefinition()
{
    int ln = cur().line;
    int defStart = ti;
    ti++;
    if (cur().t != Tk::Ident)
    {
        error(ln, "MACRO: expected name");
        skipToEndOfStatement();
        return;
    }
    Macro m;
    m.name = cur().text;
    ti++;
    while (cur().t == Tk::Ident && cur().text != "mend" && cur().text != "endm")
    {
        m.params.append(cur().text);
        ti++;
        if (cur().t == Tk::Comma) { ti++; continue; }
        break;
    }
    while (cur().t == Tk::Eol || cur().t == Tk::Colon) ti++;
    int bodyScanStart = ti;
    bool closed = false;
    while (cur().t != Tk::End)
    {
        if (cur().t == Tk::Ident && (cur().text == "mend" || cur().text == "endm"))
        {
            ti++;
            closed = true;
            break;
        }
        m.body.append(cur());
        ti++;
    }
    if (!closed)
        error(ln, QString("MACRO '%1': missing MEND").arg(m.name));
    if (macros.contains(m.name))
        error(ln, QString("MACRO '%1': duplicate definition").arg(m.name));
    macros.insert(m.name, m);
    int defEnd = ti;
    toks.remove(defStart, defEnd - defStart);
    ti = defStart;
    (void)bodyScanStart;
}

void Core::handleMacroInvocation(const Macro &m)
{
    int invocStart = ti;
    int ln = cur().line;
    ti++;
    QVector<QVector<Token>> args;
    QVector<Token> current;
    int parenDepth = 0;
    while (cur().t != Tk::Eol && cur().t != Tk::Colon && cur().t != Tk::End)
    {
        if (cur().t == Tk::LP) parenDepth++;
        else if (cur().t == Tk::RP && parenDepth > 0) parenDepth--;
        if (cur().t == Tk::Comma && parenDepth == 0)
        {
            args.append(current);
            current.clear();
            ti++;
            continue;
        }
        current.append(cur());
        ti++;
    }
    if (!current.isEmpty() || !args.isEmpty()) args.append(current);

    if (args.size() > m.params.size())
        error(ln, QString("MACRO '%1': too many arguments (%2 given, %3 expected)")
              .arg(m.name).arg(args.size()).arg(m.params.size()));

    if (totalMacroExpansions >= 100000 || toks.size() > 10000000)
    {
        error(ln, QString("MACRO '%1': expansion limit exceeded (recursive or runaway macro?)").arg(m.name));
        return;
    }
    totalMacroExpansions++;

    QVector<Token> expanded;
    expanded.reserve(m.body.size());
    for (const Token &tk : m.body)
    {
        if (tk.t == Tk::Ident)
        {
            int idx = m.params.indexOf(tk.text);
            if (idx >= 0 && idx < args.size())
            {
                for (const Token &at : args[idx]) expanded.append(at);
                continue;
            }
        }
        expanded.append(tk);
    }

    int invocEnd = ti;
    QVector<Token> tail = toks.mid(invocEnd);
    toks.resize(invocStart);
    toks.append(expanded);
    toks.append(tail);
    ti = invocStart;
}

void Core::handleConditional(const QString &id)
{
    int ln = cur().line;
    bool parentActive = condActive();

    if (id == "ifdef" || id == "ifndef")
    {
        if (cur().t != Tk::Ident)
        {
            error(ln, QString("%1: expected symbol name").arg(id.toUpper()));
            skipToEndOfStatement();
            CondFrame f; f.parentActive = parentActive; f.active = false; f.takenThisIf = true;
            condStack.append(f);
            return;
        }
        QString name = cur().text;
        ti++;
        bool defined = symbols.contains(name);
        bool cond = (id == "ifdef") ? defined : !defined;
        CondFrame f;
        f.parentActive = parentActive;
        f.active = parentActive && cond;
        f.takenThisIf = f.active;
        condStack.append(f);
        return;
    }
    if (id == "if" || id == "ifnot")
    {
        ExprResult er = parseExpr();
        bool cond = (id == "if") ? (er.value != 0) : (er.value == 0);
        CondFrame f;
        f.parentActive = parentActive;
        f.active = parentActive && cond;
        f.takenThisIf = f.active;
        condStack.append(f);
        return;
    }
    if (id == "else")
    {
        if (condStack.isEmpty())
        {
            error(ln, "ELSE without matching IF/IFDEF");
            return;
        }
        CondFrame &top = condStack.last();
        if (top.parentActive)
        {
            top.active = !top.takenThisIf;
            if (top.active) top.takenThisIf = true;
        }
        else
        {
            top.active = false;
        }
        return;
    }
    if (id == "elseif")
    {
        ExprResult er = parseExpr();
        if (condStack.isEmpty())
        {
            error(ln, "ELSEIF without matching IF/IFDEF");
            return;
        }
        CondFrame &top = condStack.last();
        if (top.parentActive && !top.takenThisIf && er.value != 0)
        {
            top.active = true;
            top.takenThisIf = true;
        }
        else
        {
            top.active = false;
        }
        return;
    }
    if (id == "endif")
    {
        if (condStack.isEmpty())
        {
            error(ln, "ENDIF without matching IF/IFDEF");
            return;
        }
        condStack.removeLast();
        return;
    }
}

void Core::parseLine()
{
    skipEols();
    if (cur().t == Tk::End) return;

    if (cur().t == Tk::Ident)
    {
        QString id = cur().text;
        if (id == "ifdef" || id == "ifndef" || id == "if" || id == "ifnot" ||
            id == "else" || id == "elseif" || id == "endif")
        {
            ti++;
            handleConditional(id);
            return;
        }
    }

    if (!condActive())
    {
        skipToEndOfStatement();
        return;
    }

    if (cur().t == Tk::Ident && cur().text == "macro" && peekTok(1).t == Tk::Ident)
    {
        handleMacroDefinition();
        return;
    }
    if (cur().t == Tk::Ident && macros.contains(cur().text))
    {
        handleMacroInvocation(macros.value(cur().text));
        return;
    }

    int startLine = cur().line;

    if (cur().t == Tk::Ident)
    {
        QString id = cur().text;
        bool hasColon = (peekTok(1).t == Tk::Colon);
        static QSet<QString> mnemonics = {
            "nop","halt","di","ei","rlca","rrca","rla","rra","daa","cpl","scf","ccf","exx",
            "neg","reti","retn","rld","rrd","ldi","ldir","ldd","lddr","cpi","cpir","cpd","cpdr",
            "ini","inir","ind","indr","outi","otir","outd","otdr",
            "ld","push","pop","jp","jr","call","ret","rst","inc","dec",
            "add","adc","sub","sbc","and","xor","or","cp","in","out","ex","im","djnz",
            "rlc","rrc","rl","rr","sla","sra","sll","srl","bit","res","set"
        };
        static QSet<QString> directives = {
            "org","equ","db","dw","dm","ds","defb","defw","defm","defs","align","assert","write",
            "read","incbin","limit","nolist","list","text","brk","code","nocode",
            "byte","word","rmem","close","str","end","print"
        };
        bool isMnem = mnemonics.contains(id) || directives.contains(id);
        (void)hasColon;

        if (!isMnem)
        {
            ti++;
            if (cur().t == Tk::Colon) ti++;

            if (cur().t == Tk::Ident && (cur().text == "equ" || cur().text == "defl"))
            {
                ti++;
                ExprResult er = parseExpr();
                if (pass == 1 && symbols.contains(id))
                    error(startLine, QString("Duplicate symbol '%1'").arg(id));
                symbols.insert(id, er.value);
                return;
            }
            if (cur().t == Tk::Eq)
            {
                ti++;
                ExprResult er = parseExpr();
                if (pass == 1 && symbols.contains(id))
                    error(startLine, QString("Duplicate symbol '%1'").arg(id));
                symbols.insert(id, er.value);
                return;
            }

            if (pass == 1 && symbols.contains(id) && symbols.value(id) != pc)
                error(startLine, QString("Duplicate symbol '%1'").arg(id));
            symbols.insert(id, pc);

            if (cur().t == Tk::Eol || cur().t == Tk::End || cur().t == Tk::Colon) return;

            if (cur().t != Tk::Ident)
            {
                errorHere("Expected mnemonic or directive after label");
                skipToEndOfStatement();
                return;
            }
        }
    }

    if (cur().t != Tk::Ident)
    {
        errorHere("Expected mnemonic or directive");
        skipToEndOfStatement();
        return;
    }

    if (cur().t == Tk::Ident && macros.contains(cur().text))
    {
        handleMacroInvocation(macros.value(cur().text));
        return;
    }

    QString word = cur().text;
    ti++;

    if (parseDirective(word)) return;
    parseInstruction(word);
}

AssemblerResult Core::run(const QString &source, const QString &base,
                          const Assembler::ProgressFn &progress)
{
    AssemblerResult r;
    basePath = base;
    progressFn = progress;
    if (progress) progress(0, 0, QStringLiteral("Lexing..."));
    Lexer lx(source);
    QString lxErr; int lxLn = 0;
    toks = lx.all(&lxErr, &lxLn);
    if (!lxErr.isEmpty())
    {
        AssemblerMessage m; m.line = lxLn; m.text = lxErr; m.isError = true;
        r.messages.append(m);
        return r;
    }

    const int tickEvery = 64;

    estimatedTokens = 0;
    if (progress)
    {
        std::function<int(const QVector<Token>&, const QString&, int)> walk;
        walk = [&](const QVector<Token> &ts, const QString &ctxDir, int depth) -> int {
            int sum = ts.size();
            if (depth >= 32) return sum;
            for (int i = 0; i + 1 < ts.size(); i++)
            {
                if (ts[i].t != Tk::Ident || ts[i].text != QStringLiteral("read")) continue;
                if (ts[i+1].t != Tk::String) continue;
                QString name = ts[i+1].text;
                QFileInfo fi(name);
                QString resolved;
                if (fi.isAbsolute()) resolved = fi.absoluteFilePath();
                else
                {
                    QString tokDir = ts[i].source.isEmpty() ? ctxDir
                                   : QFileInfo(ts[i].source).absolutePath();
                    QString cand = QDir(tokDir).absoluteFilePath(name);
                    if (!QFileInfo::exists(cand) && !basePath.isEmpty())
                        cand = QDir(basePath).absoluteFilePath(name);
                    resolved = cand;
                }
                QFile f(resolved);
                if (!f.open(QFile::ReadOnly | QFile::Text)) continue;
                QString text = QString::fromUtf8(f.readAll());
                f.close();
                Lexer slx(text, resolved);
                QString sErr; int sLn = 0;
                QVector<Token> sub = slx.all(&sErr, &sLn);
                if (!sErr.isEmpty()) continue;
                sum += walk(sub, QFileInfo(resolved).absolutePath(), depth + 1);
            }
            return sum;
        };
        estimatedTokens = walk(toks, basePath, 0);
    }

    pass = 1;
    ti = 0;
    pc = 0;
    writePc = 0;
    curLowerRom = -1;
    curUpperRom = -1;
    curRamBank  = 0xC0;
    curFile.clear();
    curToDisk = false;
    curExec = -1;
    codeEnabled = true;
    codeLimit = -1;
    limitErrorReported = false;
    endHit = false;
    condStack.clear();
    int counter = 0;
    while (!endHit && cur().t != Tk::End)
    {
        int startTi = ti;
        int startToksSize = toks.size();
        parseLine();
        if (ti == startTi && toks.size() == startToksSize) { ti++; }
        if (progress && (++counter % tickEvery) == 0)
        {
            int total = estimatedTokens > 0 ? estimatedTokens
                       : (toks.size() ? toks.size() : 1);
            int pct = int((qint64(ti) * 100) / total);
            if (pct > 100) pct = 100;
            progress(1, pct, QStringLiteral("Pass 1"));
        }
    }
    if (progress) progress(1, 100, QStringLiteral("Pass 1"));
    if (hadError) { r.messages = messages; return r; }

    messages.clear();
    pass = 2;
    ti = 0;
    pc = 0;
    writePc = 0;
    curLowerRom = -1;
    curUpperRom = -1;
    curRamBank  = 0xC0;
    curFile.clear();
    curToDisk = false;
    curExec = -1;
    codeEnabled = true;
    codeLimit = -1;
    limitErrorReported = false;
    endHit = false;
    condStack.clear();
    segments.clear();
    counter = 0;
    while (!endHit && cur().t != Tk::End)
    {
        parseLine();
        if (progress && (++counter % tickEvery) == 0)
        {
            int total = toks.size() ? toks.size() : 1;
            int pct = int((qint64(ti) * 100) / total);
            if (pct > 100) pct = 100;
            progress(2, pct, QStringLiteral("Pass 2"));
        }
    }
    if (progress) progress(2, 100, QStringLiteral("Pass 2"));

    r.messages = messages;
    r.segments = segments;
    r.ok = !hadError;
    return r;
}

} // namespace

AssemblerResult Assembler::Assemble(const QString &source, const QString &basePath,
                                    const ProgressFn &progress)
{
    Core c;
    return c.run(source, basePath, progress);
}
