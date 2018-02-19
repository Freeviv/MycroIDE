#include "pythonsourceedit.h"

#include <QKeyEvent>
#include <QStringRef>
#include <QTextBlock>

#include <cstdio>

PythonSourceEdit::PythonSourceEdit(QWidget *parent)
    : QTextEdit(parent)
{
    intendDepth.push_back(0);
}

void PythonSourceEdit::checkIntendDepth()
{
    QStringList lines = this->toPlainText().split('\n');
    intendDepth.clear();
    intendDepth.reserve(lines.size());
    for(int i = 0; i < lines.size(); ++i)
    {
        QString line = lines.at(i);
        int whitespaces = 0;
        for(int j = 0; j < line.size(); ++j)
        {
            if(line.at(i) == ' ')whitespaces++;
        }
        intendDepth.replace(i,(whitespaces / 4) + ((whitespaces % 4 == 0) ? 0 : 1) );
    }
}

int PythonSourceEdit::getCurrentLine() const
{
    QTextCursor cursor = this->textCursor();

    int lines = 0;
    while(cursor.position() - cursor.positionInBlock() > 0) {
        cursor.movePosition(QTextCursor::Up);
        lines++;
    }
    QTextBlock block = cursor.block().previous();

    while(block.isValid()) {
        lines += block.lineCount();
        block = block.previous();
    }
    return lines;
}

void PythonSourceEdit::keyPressEvent(QKeyEvent *e)
{
    //TODO inteddepth on removal and refresh intend depth on tab
    if(e->key() == Qt::Key_Tab)
    {
        this->textCursor().insertText("    ");
        int line = getCurrentLine();
        intendDepth.replace(line,intendDepth.at(line) + 1);
        e->accept();
        return;
    }
    if( e->key() == Qt::Key_Backspace)
    {
        int pos = this->textCursor().position();
        if(pos < 4)
        {
            QTextEdit::keyPressEvent(e);
            return;
        }
        QString tmp = this->toPlainText();
        QStringRef temp = QStringRef(&tmp,pos - 4, 4);
        if(temp.trimmed().isEmpty())
        {
            for(int i = 0; i < 4; ++i)
            {
                this->textCursor().deletePreviousChar();
            }
            int line = getCurrentLine();
            int depth = intendDepth.at(line);
            if(depth > 0)intendDepth.replace(line,depth - 1);
        }
        else
        {
            QTextEdit::keyPressEvent(e);
            return;
        }
        e->accept();
        return;
    }
    if(e->key() == Qt::Key_Delete)
    {
        int pos = this->textCursor().position();
        QString tmp = this->toPlainText();
        if(pos + 4 >= tmp)
        {
            QTextEdit::keyPressEvent(e);
            return;
        }
        QStringRef temp = QStringRef(&tmp,pos, 4);
        if(temp.trimmed().isEmpty())
        {
            for(int i = 0; i < 4; ++i)
            {
                this->textCursor().deleteChar();
            }
        }
        else
        {
            QTextEdit::keyPressEvent(e);
            return;
        }
        e->accept();
        return;
    }
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        // TODO check intend depth
        // lookup previous line to check if it ends with a ':'
        int line = getCurrentLine();
        int depth = intendDepth.at(line);

        QString tmp = this->toPlainText();
        if(this->textCursor().position() > 0 &&
          tmp.at(this->textCursor().position() - 1) == ':')
        {
            depth++;
        }
        else
        {
            int pos = this->textCursor().position();
            int pos_b = this->textCursor().positionInBlock();
            QString line = QStringRef(&tmp,pos - pos_b,pos_b).toString().trimmed();
            if(line.endsWith(QString("pass")) ||
               line.startsWith(QString("raise")) ||
               line.startsWith(QString("return")))
            {
                if(depth > 0)depth--;
            }
        }

        intendDepth.insert(line + 1,depth);
        // at the end execute normal return
        QTextEdit::keyPressEvent(e);
        for(int i = 0; i < depth; ++i)
        {
            this->textCursor().insertText("    ");
        }
        return;
    }
    QTextEdit::keyPressEvent(e);
}
