#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QString>
#include <QtSerialPort/QSerialPort>

class Serial : public QObject
{
    Q_OBJECT
public:
    Serial(QString port);
    ~Serial();
    void setBaudrate(unsigned int rate);
    bool open();
    bool isOpen();
    void close();
    qint64 read(char *data,qint64 maxSize);
    QString readLine();
    qint64 write(char *data,qint64 maxSize);
    void writeLine(QString line);
    bool flush();

private:
    QString m_port_path;
    QSerialPort *m_port;
    unsigned int m_baudrate;

private slots:
    void slot_data_available();

signals:
    void signal_data_available(int num_bytes);
};

#endif // SERIAL_H
