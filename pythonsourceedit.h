#ifndef PYTHONSOURCEEDIT_H
#define PYTHONSOURCEEDIT_H

#include <QTextEdit>

class PythonSourceEdit : public QTextEdit
{
    Q_OBJECT
public:
    PythonSourceEdit(QWidget *parent = 0);

private:
    void keyPressEvent(QKeyEvent *e);
};

#endif // PYTHONSOURCEEDIT_H
