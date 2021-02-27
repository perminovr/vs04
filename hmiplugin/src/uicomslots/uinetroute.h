#ifndef UINETROUTE_H
#define UINETROUTE_H

#include "uinetaddress.h"

class UINetRoute : public UINetAddress
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)
	Q_PROPERTY(QString gateway READ gateway WRITE setGateway NOTIFY gatewayChanged)
	Q_PROPERTY(UINetAddress* src READ src WRITE setSrc NOTIFY srcChanged)

public:
	UINetRoute(QObject *parent = nullptr);
	virtual ~UINetRoute();

	QString gateway();
	UINetAddress *src();

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterType<UINetRoute>(pkgName, mj, mi, "UINetRoute");
	}

public slots:
	virtual void setIface(Interface) override;
	void setGateway(const QString &);
	void setSrc(UINetAddress*);

signals:
	void gatewayChanged();
	void srcChanged();

private slots:
	void doSetup();
	void netAddr_onPropertyChanged();

protected:
	QString m_gw;
	UINetAddress *m_src;
	virtual void setSelf() override;
};

#endif // UINETROUTE_H
