#ifndef PYTHONSOURCEEDIT_H
#define PYTHONSOURCEEDIT_H

#include <QTextEdit>
#include <QVector>

class PythonSourceEdit : public QTextEdit
{
    Q_OBJECT
public:
    PythonSourceEdit(QWidget *parent = 0);

private:
    QVector<uint8_t> intendDepth;
    void checkIntendDepth();
    int getCurrentLine() const;
    void keyPressEvent(QKeyEvent *e);
};

#endif // PYTHONSOURCEEDIT_H
