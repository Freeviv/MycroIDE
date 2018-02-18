#include "pythonsourceedit.h"

#include <QStringRef>
#include <QKeyEvent>

PythonSourceEdit::PythonSourceEdit(QWidget *parent)
    : QTextEdit(parent)
{

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
        e->accept();
        return;
    }
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        // TODO check intend depth
    }
    QTextEdit::keyPressEvent(e);
}
