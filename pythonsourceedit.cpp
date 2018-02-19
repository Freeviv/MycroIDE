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
    if(e->key() == Qt::Key_Tab)
    {
        this->textCursor().insertText("    ");
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
        fprintf(stderr,"%i\n",getCurrentLine());
        // at the end execute normal return
        QTextEdit::keyPressEvent(e);
        return;
    }
    QTextEdit::keyPressEvent(e);
}
