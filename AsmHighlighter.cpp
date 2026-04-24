#include "AsmHighlighter.h"

#include <QColor>

AsmHighlighter::AsmHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    QTextCharFormat labelFormat;
    labelFormat.setForeground(QColor("#7F5000"));
    labelFormat.setFontWeight(QFont::Bold);

    QTextCharFormat mnemonicFormat;
    mnemonicFormat.setForeground(QColor("#4080F0"));
    mnemonicFormat.setFontWeight(QFont::Bold);

    QTextCharFormat directiveFormat;
    directiveFormat.setForeground(QColor("#aB2056"));
    directiveFormat.setFontWeight(QFont::Bold);

    QTextCharFormat registerFormat;
    registerFormat.setForeground(QColor("#20A0A0"));

    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor("#B87333"));

    stringFormat.setForeground(QColor("#A31515"));

    commentFormat.setForeground(QColor("#008000"));
    commentFormat.setFontItalic(true);

    rules.append({QRegularExpression(QStringLiteral("^\\s*[A-Za-z_.@][A-Za-z0-9_.?]*(?=\\s*:)")),
                  labelFormat});

    auto wb = [](const QStringList &words) {
        return QStringLiteral("\\b(?:") + words.join('|') + QStringLiteral(")\\b");
    };

    QStringList mnemonics = {
        "nop","halt","di","ei","rlca","rrca","rla","rra","daa","cpl","scf","ccf","exx",
        "neg","reti","retn","rld","rrd","ldi","ldir","ldd","lddr","cpi","cpir","cpd","cpdr",
        "ini","inir","ind","indr","outi","otir","outd","otdr",
        "ld","push","pop","jp","jr","call","ret","rst","inc","dec",
        "add","adc","sub","sbc","and","xor","or","cp","in","out","ex","im","djnz",
        "rlc","rrc","rl","rr","sla","sra","sll","srl","bit","res","set"
    };
    QRegularExpression mnemExpr(wb(mnemonics),
                                QRegularExpression::CaseInsensitiveOption);
    rules.append({mnemExpr, mnemonicFormat});

    QStringList directives = {
        "org","equ","db","dw","dm","ds","defb","defw","defm","defs",
        "align","assert","write","read","incbin"
    };
    QRegularExpression dirExpr(wb(directives),
                               QRegularExpression::CaseInsensitiveOption);
    rules.append({dirExpr, directiveFormat});

    QStringList registers = {
        "af","bc","de","hl","ix","iy","sp","ixh","ixl","iyh","iyl",
        "a","b","c","d","e","h","l","f","i","r"
    };
    QRegularExpression regExpr(wb(registers),
                               QRegularExpression::CaseInsensitiveOption);
    rules.append({regExpr, registerFormat});

    QString numberPattern =
        QStringLiteral("(?:&[xX][01]+|&[0-9A-Fa-f]+|#[0-9A-Fa-f]+|\\$[0-9A-Fa-f]+|")
        + QStringLiteral("%[01]+|0[xX][0-9A-Fa-f]+|0[bB][01]+|")
        + QStringLiteral("\\b\\d[0-9A-Fa-f]*[Hh]\\b|\\b[01]+[Bb]\\b|\\b\\d+\\b)");
    rules.append({QRegularExpression(numberPattern), numberFormat});

    stringExpr = QRegularExpression(QStringLiteral("\"(?:[^\"\\\\]|\\\\.)*\""));
    charExpr   = QRegularExpression(QStringLiteral("'(?:[^'\\\\]|\\\\.)*'"));
    commentExpr = QRegularExpression(QStringLiteral(";[^\\n]*"));
}

void AsmHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &r : rules)
    {
        QRegularExpressionMatchIterator it = r.pattern.globalMatch(text);
        while (it.hasNext())
        {
            QRegularExpressionMatch m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), r.format);
        }
    }
    for (QRegularExpression *e : {&stringExpr, &charExpr})
    {
        QRegularExpressionMatchIterator it = e->globalMatch(text);
        while (it.hasNext())
        {
            QRegularExpressionMatch m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), stringFormat);
        }
    }
    QRegularExpressionMatch cm = commentExpr.match(text);
    if (cm.hasMatch())
        setFormat(cm.capturedStart(), cm.capturedLength(), commentFormat);
}
