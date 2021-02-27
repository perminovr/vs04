#include "uiserialport.h"
#include "uitypeconverter.h"

#define scast(t,v) static_cast<t>(v)


QStringList UISerialPort::openedPorts = {};


void UISerialPort::classBegin()
{}


void UISerialPort::componentComplete()
{
	this->m_complete = true;
	if (openedPorts.contains(m_dev)) {
		#if PLC_BUILD
			lError(provider) << "Couldn't create serial port with name \"" << m_dev << "\": already exists";
		#endif
		return;
	}
	openedPorts.push_back(m_dev);
	emit completed(this);
}


bool UISerialPort::isComplete()
{
	return this->m_complete;
}


UISerialPort::UISerialPort(QObject *parent) : QObject(parent)
{
	#if PLC_BUILD
		provider = new SerialPortProvider(this);
	#endif
	m_complete = false;
	m_baudRate = 0;
	m_parity = 0;
	m_dataBits = 0;
	m_stopBits = 0;
}


UISerialPort::~UISerialPort()
{
	qDebug() << "~UISerialPort";
	openedPorts.removeOne(m_dev);
}
