#ifndef HMINETWORKING_H
#define HMINETWORKING_H

#include "hminetworkingcommon.h"

using namespace HMINetworkingCommon;

class HMISystemNetworking;
class NetworkingProvider;

class HMINetworkingProvider : public QObject
{
	Q_OBJECT
public:
	struct AddressArgs {
		QString ip;
		QString mask;
		Iface iface;
		AddressArgs() {
			iface = ndefIface;
		}
	};
	void *setAddress(void *paddr, const AddressArgs &args, QString *error = nullptr);
	void delAddress(void *paddr);
	struct RouteArgs {
		QString netIp;
		QString netMask;
		QString gateway;
		QString src;
		Iface iface;
		RouteArgs() {
			iface = ndefIface;
		}
	};
	void *setRoute(void *proute, const RouteArgs &args, QString *error = nullptr);
	void delRoute(void *proute);

	void ifaceState(Iface iface, bool &available, bool &link) const;
	void serviceIpMode(Iface iface, IPMode &mode) const;
	bool systemAddress(Iface iface, QString &ip, QString &mask) const;
	bool systemRoute(Iface iface, QString &gateway) const;
	NetMode netMode() const;

	HMINetworkingProvider(QObject *parent = nullptr);
	virtual ~HMINetworkingProvider();

public slots:
	void setIfaceState(Iface iface, bool up);

signals:
	void ifaceStateChanged(Iface iface);
	void serviceIpModeChanged(Iface iface);
	void systemAddressChanged(Iface iface);
	void systemRouteChanged(Iface iface);
	void netModeChanged(NetMode mode);

private:
	NetworkingProvider *provider;
	HMISystemNetworking *system;
};

#endif // HMINETWORKING_H
