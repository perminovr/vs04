#ifndef UINETADDRESS_H
#define UINETADDRESS_H

#include <QObject>
#include <QtQml>
#include <QQmlParserStatus>

#if PLC_BUILD
#   include "hminetworkingprovider.h"
#   include "logcommon.h"
#endif

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(type t) { if (this->m_##name != t) { this->m_##name = t; setSelf(); emit this->notifier(); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
	protected: type m_##name;

class UINetAddress : public QObject , public QQmlParserStatus
		#if PLC_BUILD
		, public LoggingBase
		#endif
{
public:
	enum Interface {
		Ethernet,
		Wifi
	};

	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

	Q_PROPERTY(QString ip READ ip WRITE setIp NOTIFY ipChanged)
	Q_PROPERTY(QString mask READ mask WRITE setMask NOTIFY maskChanged)
	Q_PROPERTY(UINetAddress::Interface iface READ iface WRITE setIface NOTIFY ifaceChanged)
	Q_PROPERTY(bool isComplete READ isComplete NOTIFY completed)
	Q_PROPERTY_IMPLEMENTATION(QString , ip , ip , setIp , ipChanged)
	Q_PROPERTY_IMPLEMENTATION(QString , mask , mask , setMask , maskChanged)
	Q_ENUMS(Interface)

public:
	UINetAddress(QObject *parent = nullptr);
	virtual ~UINetAddress();

	virtual void classBegin() override;
	virtual void componentComplete() override;
	bool isComplete();

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterType<UINetAddress>(pkgName, mj, mi, "UINetAddress");
	}

	Interface iface() const;

public slots:
	virtual void setIface(Interface);

signals:
	void ifaceChanged();
	void completed(UINetAddress *);

private slots:

protected:
	#if PLC_BUILD
		HMINetworkingProvider *networking;
	#endif
	void *self;
	bool m_complete;
	Interface m_iface;
	virtual void setSelf();
};

#endif // UINETADDRESS_H
