#include "pythonhighlighter.h"

#include <QTextDocument>

#include <QDebug>

PythonHighlighter::PythonHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    rule.highlighting_type = KEYWORD;
    QStringList keywordPatterns;
    keywordPatterns << "\\bFalse\\b" << "\\bNone\\b" << "\\bTrue\\b"
                    << "\\band\\b"  << "\\bas\\b" << "\\bassert\\b"
                    << "\\bbreak\\b" << "\\bclass\\b" << "\\bcontinue\\b"
                    << "\\bdef\\b" << "\\bdel\\b" << "\\belif\\b"
                    << "\\belse\\b" << "\\bexcept\\b" << "\\bfinally\\b"
                    << "\\bfor\\b" << "\\bfrom\\b" << "\\bglobal\\b"
                    << "\\bof\\b" << "\\bimport\\b" << "\\bin\\b"
                    << "\\bis\\b" << "\\blambda\\b" << "\\bnonlocal\\b"
                    << "\\bnot\\b" << "\\bor\\b" << "\\bpass\\b"
                    << "\\braise\\b" << "\\breturn\\b" << "\\btry\\b"
                    << "\\bwhile\\b" << "\\bwith\\b" << "\\byield\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::gray);
    rule.highlighting_type = SINGLELINE_COMMENT;
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.highlighting_type = MULTILINE_COMMENT;
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.highlighting_type = FUNCTION;
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    selfFormat.setFontItalic(true);
    selfFormat.setForeground(QBrush(QColor(224,114,33)));
    rule.highlighting_type = SELF;
    rule.pattern = QRegularExpression("\\s*sel[f]");
    rule.format = selfFormat;
    highlightingRules.push_front(rule);

    whitespaces.setBackground(QBrush(QColor(140,140,240)));
    rule.highlighting_type = WHITESPACES;
    rule.pattern = QRegularExpression("\\s");
    rule.format = whitespaces;
    highlightingRules.append(rule);

    mls = QRegularExpression("(['\"])\\1\\1");
    mle = QRegularExpression(".*(['\"])\\1\\1");
    highlight_level = KEYWORD | SINGLELINE_COMMENT | MULTILINE_COMMENT | FUNCTION | SELF;
}

void PythonHighlighter::markWhitespaces(bool mark)
{
    if(mark)
    {
        highlight_level = highlight_level | WHITESPACES;
    }
    else
    {
        highlight_level = highlight_level ^ WHITESPACES;
    }
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        if(highlight_level & rule.highlighting_type)
        {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

    setCurrentBlockState(0);
    int start = text.indexOf(mls);
    if(start != -1)
    {
        if(previousBlockState() == 1)
        {
            setFormat(0,start + 3,multiLineCommentFormat);
        }
        else
        {
            setFormat(start,text.length() - start,multiLineCommentFormat);
            setCurrentBlockState(1);
        }
    }
    else
    {
        if(previousBlockState() == 1)
        {
            setCurrentBlockState(1);
            setFormat(0,text.length(),multiLineCommentFormat);
        }
    }
}
