#ifndef HMISYSTEMNETWORKING_H
#define HMISYSTEMNETWORKING_H

#include "hminetworkingcommon.h"

using namespace HMINetworkingCommon;

class NetworkingProvider;
class SystemNetworking;
class HMINetworkingProvider;
class IWifiStation;

class HMISystemNetworking : public QObject
{
	Q_OBJECT
public:
	static HMISystemNetworking *instance(QObject *object);

	NetMode netMode() const;

	void ifaceState(Iface iface, bool &available, bool &link) const;
	IPMode ifaceSysIpMode(Iface iface) const;

	bool address(Iface iface, QNetworkAddressEntry &addr) const;
	bool route(Iface iface, QHostAddress &gateway) const;

	QString getMac(Iface iface) const;

	void setWifiProvider(IWifiStation *wifi);

	void cleanUp();

public slots:
	void setNetMode(NetMode mode);

	void setIfaceLink(Iface iface, bool up);
	void setSysIfaceIpMode(Iface iface, IPMode mode);

	void setAddress(Iface iface, const QNetworkAddressEntry &addr, QString *error = nullptr);
	void setRoute(Iface iface, const QHostAddress &gateway, QString *error = nullptr);

signals:
	void netModeChanged(NetMode mode);

	void ifaceStateChanged(Iface iface);
	void ifaceSysIpModeChanged(Iface iface, IPMode mode);

	void addressChanged(Iface iface);
	void routeChanged(Iface iface);

private:
	HMISystemNetworking(QObject *parent = nullptr);
	virtual ~HMISystemNetworking();
	Q_DISABLE_COPY(HMISystemNetworking)

	friend class HMINetworkingProvider;

	static HMISystemNetworking *snw_instance;

	NetMode m_netMode;

	const QObject *object;
	SystemNetworking *system;
	NetworkingProvider *provider;

	IWifiStation *wifi;
	void handleWifiState();
};

#endif // HMISYSTEMNETWORKING_H
