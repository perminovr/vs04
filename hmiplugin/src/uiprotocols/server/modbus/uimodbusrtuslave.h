#ifndef UIMODBUSRTUSLAVE_H
#define UIMODBUSRTUSLAVE_H

#include "uimodbusslave.h"
#include "uiserialport.h"
#if PLC_BUILD
#   include "modbus/modbusrtuslaveprovider.h"
#endif

class UIModbusRtuSlave : public UIModbusSlave
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)
	Q_PROPERTY(UISerialPort *port READ port WRITE setPort NOTIFY portChanged)

public:
	UIModbusRtuSlave(QObject *parent = nullptr);
	virtual ~UIModbusRtuSlave();

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterType<UIModbusRtuSlave>(pkgName, mj, mi, "UIModbusRtuSlave");
	}

	UISerialPort *port();

public slots:
	// from qml to core
	virtual void restart() override;

	void setPort(UISerialPort *port);

signals:
	void portChanged();

protected:
	UISerialPort *m_port;

private slots:
	IF_PLC_BUILD(void applyAndRestart(ModbusRtuSlaveProvider::Config &cfg));
	void onCompleted(UISerialPort *port);
	void port_onPropertyChanged();
};

#endif // UIMODBUSRTUSLAVE_H
