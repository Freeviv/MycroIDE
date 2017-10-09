#include "serial.h"

#include <QByteArray>
#include <QTimer>


Serial::Serial(QString port)
{
    m_port_path = port;
    m_port = new QSerialPort(this);
    connect(m_port,SIGNAL(readyRead()),SLOT(slot_data_available()));
    connect(m_port,SIGNAL(errorOccurred(QSerialPort::SerialPortError)),SLOT(slot_device_error(QSerialPort::SerialPortError)));
}

Serial::~Serial()
{
    close();
}

void Serial::slot_data_available()
{
    emit signal_data_available(m_port->bytesAvailable());
}

void Serial::slot_device_error(QSerialPort::SerialPortError error)
{
    if(error & QSerialPort::NoError)
        return;
    emit signal_device_error(error);
}

void Serial::setBaudrate(unsigned int rate)
{
    m_baudrate = rate;
}

bool Serial::open()
{
    m_port->setPortName(m_port_path);
    m_port->open(QIODevice::ReadWrite);
    m_port->setBaudRate(m_baudrate);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    return m_port->isOpen() && m_port->isWritable() && m_port->isReadable();
}

bool Serial::isOpen()
{
    return m_port->isOpen();
}

void Serial::close()
{
    m_port->close();
}

qint64 Serial::read(char *data,qint64 maxSize)
{
    return m_port->read(data,maxSize);
}

QString Serial::readLine()
{
    QByteArray data = m_port->readAll();
    return QString::fromStdString(data.toStdString());
}

qint64 Serial::write(char *data, qint64 maxSize)
{
    return m_port->write(data,maxSize);
}

void Serial::writeLine(QString line)
{
    QByteArray data = line.toUtf8();
    //TODO blocking send
}

bool Serial::flush()
{
    return m_port->flush();
}
