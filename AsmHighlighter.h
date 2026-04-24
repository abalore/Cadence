#ifndef ASMHIGHLIGHTER_H
#define ASMHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

class AsmHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit AsmHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<Rule> rules;
    QTextCharFormat commentFormat;
    QTextCharFormat stringFormat;
    QRegularExpression commentExpr;
    QRegularExpression stringExpr;
    QRegularExpression charExpr;
};

#endif // ASMHIGHLIGHTER_H
