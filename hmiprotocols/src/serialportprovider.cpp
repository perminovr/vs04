#include "serialportprovider.h"
#include <QSerialPort>

#define scast(t,v) static_cast<t>(v)


QString SerialPortProvider::deviceByNumber(char number)
{
	return QString("/dev/ttyS%1").arg(number+1); // todo
}


QString SerialPortProvider::defaultDevice()
{
	return deviceByNumber('1');
}


SerialPortProvider::SerialPortProvider(QObject *parent) : ProtocolProvider(parent)
{
	INIT_LOGGER("serial");
	m_dev = defaultDevice();
	m_baudRate = scast(int, QSerialPort::BaudRate::Baud9600);
	m_parity = scast(int, QSerialPort::Parity::NoParity);
	m_dataBits = scast(int, QSerialPort::DataBits::Data8);
	m_stopBits = scast(int, QSerialPort::StopBits::OneStop);
}


SerialPortProvider::~SerialPortProvider()
{
	qDebug() << "~SerialPortProvider";
}

