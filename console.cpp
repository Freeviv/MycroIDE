#include "console.h"

#include <QScrollBar>

Console::Console(QWidget *parent)
    : QPlainTextEdit(parent)
{
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    setPalette(p);
    connect(document(),SIGNAL(contentsChange(int,int,int)),SLOT(text_changed(int,int,int)));
}

void Console::putData(const QChar &data)
{
    ignore_next_change = true;
    insertPlainText(QString(data));
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::putData(const QByteArray &data)
{
    ignore_next_change = true;
    insertPlainText(QString(data));
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::keyPressEvent(QKeyEvent *e)
{
    // TODO send correct bytes for del, left ...
    switch (e->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        break;
    default:
        //qDebug(e->text().toStdString().c_str());
        emit external_text_change(e->text());
        QPlainTextEdit::keyPressEvent(e);
    }
}

void Console::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}

void Console::text_changed(int pos, int del, int add)
{
    if(ignore_next_change)
    {
        ignore_next_change = false;
        return;
    }
    QString added = toPlainText().mid(pos,add);
    for(int i = 0; i < added.length(); ++i)
        textCursor().deletePreviousChar();
    if(!added.isEmpty())
        qDebug(added.toStdString().c_str());
}
