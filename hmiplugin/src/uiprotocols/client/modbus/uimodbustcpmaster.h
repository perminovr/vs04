#ifndef UIMODBUSTCPMASTER_H
#define UIMODBUSTCPMASTER_H

#include "uimbregister.h"
#include "uimodbusmaster.h"
#include "uinetaddress.h"
#if PLC_BUILD
#   include "modbus/modbustcpmasterprovider.h"
#endif

class UIModbusTcpMaster : public UIModbusMaster
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

	Q_PROPERTY(QString ip READ ip WRITE setIp NOTIFY ipChanged)
	Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(UINetAddress *srcAddr READ srcAddr WRITE setSrcAddr NOTIFY srcAddrChanged)

public:
	UIModbusTcpMaster(QObject *parent = nullptr);
	virtual ~UIModbusTcpMaster();

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterType<UIModbusTcpMaster>(pkgName, mj, mi, "UIModbusTcpMaster");
	}

	QString ip();
	int port();
    UINetAddress *srcAddr();

public slots:
	// from qml to core
	virtual void restart() override;

	void setIp(const QString &);
	void setPort(int );
    void setSrcAddr(UINetAddress *);

signals:
	void ipChanged();
	void portChanged();
    void srcAddrChanged();

protected:
	QString m_ip;
	int m_port;
    UINetAddress *m_srcAddr;

private slots:
    void applyAndRestart();
    void onCompleted(UINetAddress *src);

    void srcAddr_onPropertyChanged();
};

#endif // UIMODBUSTCPMASTER_H
