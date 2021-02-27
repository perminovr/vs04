#ifndef UIMODBUSRTUMASTER_H
#define UIMODBUSRTUMASTER_H

#include "uimodbusmaster.h"
#include "uiserialport.h"
#if PLC_BUILD
#   include "modbus/modbusrtumasterprovider.h"
#endif

class UIModbusRtuMaster : public UIModbusMaster
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)
	Q_PROPERTY(UISerialPort *port READ port WRITE setPort NOTIFY portChanged)

public:
	UIModbusRtuMaster(QObject *parent = nullptr);
	virtual ~UIModbusRtuMaster();

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterType<UIModbusRtuMaster>(pkgName, mj, mi, "UIModbusRtuMaster");
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
	IF_PLC_BUILD(void applyAndRestart(ModbusRtuMasterProvider::Config &cfg));
	void onCompleted(UISerialPort *port);
    void port_onPropertyChanged();
};

#endif // UIMODBUSRTUMASTER_H
