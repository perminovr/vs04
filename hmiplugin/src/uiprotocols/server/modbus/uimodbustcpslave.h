#ifndef UIMODBUSTCPSLAVE_H
#define UIMODBUSTCPSLAVE_H

#include "uimodbusslave.h"
#include "uinetaddress.h"

// #define UIProtocolClassName UIModbusTcpSlave
// #include "uiprotocolctl.h"

#if PLC_BUILD
#   include "modbus/modbustcpslaveprovider.h"
#endif


class UIModbusTcpSlave : public UIModbusSlave
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

	Q_PROPERTY(UINetAddress *netAddr READ netAddr WRITE setNetAddr NOTIFY netAddrChanged)
	Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

public:
	UIModbusTcpSlave(QObject *parent = nullptr);
	virtual ~UIModbusTcpSlave();

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterType<UIModbusTcpSlave>(pkgName, mj, mi, "UIModbusTcpSlave");
	}

	UINetAddress *netAddr();
	int port();

public slots:
	// from qml to core
	virtual void restart() override;

	void setNetAddr(UINetAddress *);
	void setPort(int );

signals:
	void netAddrChanged();
	void portChanged();

protected:
	UINetAddress *m_netAddr;
	int m_port;

private slots:
    void applyAndRestart(const QString &ip);
    void onCompleted(UINetAddress *addr);

	void netAddr_onPropertyChanged();

private:
	// UIProtocolCtlCfgClass_Declare
};

#endif // UIMODBUSTCPSLAVE_H
