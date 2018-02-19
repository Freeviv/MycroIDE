#ifndef PYTHONHIGHLIGHTER_H
#define PYTHONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

enum highlighting_type
{
    KEYWORD = 1,
    SINGLELINE_COMMENT = 2,
    MULTILINE_COMMENT = 4,
    FUNCTION = 8,
    WHITESPACES = 16,
    SELF = 32
};

class PythonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    PythonHighlighter(QTextDocument *parent = 0);
    void markWhitespaces(bool mark);

protected:
    void highlightBlock(const QString &text) override;

private:
    uint64_t highlight_level;
    QChar ml_char;
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
        uint64_t highlighting_type;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression mls;
    QRegularExpression mle;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat selfFormat;

    QTextCharFormat whitespaces;

    void do_multiline_highlights();
};

#endif // PYTHONHIGHLIGHTER_H
