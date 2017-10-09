#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>

class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    Console(QWidget *parent);
    void putData(const QChar &data);
    void putData(const QByteArray &data);
    void delete_last_char();

protected:
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

private:
    bool ignore_next_change;

private slots:
    void text_changed(int pos, int del, int add);

signals:
    void external_text_change(QString change);
};

#endif // CONSOLE_H
