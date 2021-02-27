#ifndef UISERIALPORT_H
#define UISERIALPORT_H

#include <QObject>
#include <QtQml>
#include <QQmlParserStatus>

#if PLC_BUILD
#   include "serialportprovider.h"
#endif

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(type t) { if (this->m_##name != t) { this->m_##name = t; emit this->notifier(); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
	private: type m_##name;

class UISerialPort : public QObject , public QQmlParserStatus
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

	Q_PROPERTY(QString dev READ dev WRITE setDev NOTIFY devChanged)
	Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)
	Q_PROPERTY(int parity READ parity WRITE setParity NOTIFY parityChanged)
	Q_PROPERTY(int dataBits READ dataBits WRITE setDataBits NOTIFY dataBitsChanged)
	Q_PROPERTY(int stopBits READ stopBits WRITE setStopBits NOTIFY stopBitsChanged)
	Q_PROPERTY(bool isComplete READ isComplete NOTIFY completed)
	Q_PROPERTY_IMPLEMENTATION(QString , dev , dev , setDev , devChanged)
	Q_PROPERTY_IMPLEMENTATION(int , baudRate , baudRate , setBaudRate , baudRateChanged)
	Q_PROPERTY_IMPLEMENTATION(int , parity , parity , setParity , parityChanged)
	Q_PROPERTY_IMPLEMENTATION(int , dataBits , dataBits , setDataBits , dataBitsChanged)
	Q_PROPERTY_IMPLEMENTATION(int , stopBits , stopBits , setStopBits , stopBitsChanged)

public:
	UISerialPort(QObject *parent = nullptr);
	virtual ~UISerialPort();

	virtual void classBegin() override;
	virtual void componentComplete() override;
	bool isComplete();

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterType<UISerialPort>(pkgName, mj, mi, "UISerialPort");
	}

public slots:

signals:
	void completed(UISerialPort *);

protected:
	static QStringList openedPorts;

private slots:

private:
	#if PLC_BUILD
		SerialPortProvider *provider;
	#endif
	bool m_complete;
};

#endif // UISERIALPORT_H
