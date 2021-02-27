#ifndef SERIALPORTPROVIDER_H
#define SERIALPORTPROVIDER_H

#include "protocolprovider.h"
#include <QObject>

class QSerialPort;

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(type t) { if (this->m_##name != t) { this->m_##name = t; emit this->notifier(); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
	private: type m_##name;

class SerialPortProvider : public ProtocolProvider
{
	Q_OBJECT
	Q_PROPERTY(QString dev READ dev WRITE setDev NOTIFY devChanged)
	Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
	Q_PROPERTY(int parity READ parity WRITE setParity NOTIFY parityChanged)
	Q_PROPERTY(int dataBits READ dataBits WRITE setDataBits NOTIFY dataBitsChanged)
	Q_PROPERTY(int stopBits READ stopBits WRITE setStopBits NOTIFY stopBitsChanged)
	Q_PROPERTY_IMPLEMENTATION(QString , dev , dev , setDev , devChanged)
	Q_PROPERTY_IMPLEMENTATION(int , baudRate , baudRate , setBaudRate , baudRateChanged)
	Q_PROPERTY_IMPLEMENTATION(int , parity , parity , setParity , parityChanged)
	Q_PROPERTY_IMPLEMENTATION(int , dataBits , dataBits , setDataBits , dataBitsChanged)
	Q_PROPERTY_IMPLEMENTATION(int , stopBits , stopBits , setStopBits , stopBitsChanged)

public:
	SerialPortProvider(QObject *parent = nullptr);
	virtual ~SerialPortProvider();

	static QString defaultDevice();
	static QString deviceByNumber(char number);

public slots:
    virtual void restart(const ProtocolProvider::Config *config) override {Q_UNUSED(config)}
	virtual bool pause() override {return false;}
	virtual bool resume() override {return false;}

signals:

private:
	QSerialPort *serial;
};

#endif // SERIALPORTPROVIDER_H
