
#include "systemnetworking.h"
#include "networkingprovider.h"
#include "iwifistation.h"
#include "udhcpprovider.h"
#include "platform.h"

#include "hmisystemnetworking.h"
#include "hminetworkingprovider.h"

#define scast(t,v) static_cast<t>(v)


namespace HMINetworkingCommon
{
Iface ifaceByName(const QString &iface) {
	const auto ifaceList = getIfacesList();
	int cnt = 0;
	for (const auto &x : ifaceList) {
		if (iface == x) return scast(Iface, cnt);
		cnt++;
	}
	return Iface::ndefIface;
}
QString ifaceName(Iface iface) {
	const auto ifaceList = getIfacesList();
	int idx = scast(int, iface);
	if (idx >= 0 && idx < ifaceList.size())
		return ifaceList[idx];
	return QStringLiteral("");
}
} // HMINetworkingCommon


static inline HMINetworkingCommon::IPMode ipMode(NetworkingCommon::IPMode mode)
{
	switch (mode) {
		case NetworkingCommon::Dhcp: return HMINetworkingCommon::Dhcp;
		default: return HMINetworkingCommon::Static;
	}
}
static inline NetworkingCommon::IPMode ipMode(HMINetworkingCommon::IPMode mode)
{
	switch (mode) {
		case HMINetworkingCommon::Dhcp: return NetworkingCommon::Dhcp;
		default: return NetworkingCommon::Static;
	}
}


// ##################################################################################################
// ######################################### HMISystemNetworking ####################################
// ##################################################################################################


void HMISystemNetworking::handleWifiState()
{
	if (this->wifi) {
		QString iname = ifaceName(Iface::WifiIface);
		bool wifi_av, wifi_link;
		this->provider->ifaceState(iname, wifi_av, wifi_link);
		if (wifi_av && (this->m_netMode == NetMode::Wifi || this->m_netMode == NetMode::EthWifi)) {
			this->wifi->restart(iname);
		} else {
			this->wifi->stop();
		}
	}
}


void HMISystemNetworking::setWifiProvider(IWifiStation *wifi)
{
	this->wifi = wifi;
	handleWifiState();
}


void HMISystemNetworking::setNetMode(NetMode mode)
{
	if (m_netMode != mode) {
		m_netMode = mode;
		switch (mode) {
		case NetMode::Ethernet:
			setIfaceLink(Iface::EthIface, true);
			setIfaceLink(Iface::WifiIface, false);
			break;
		case NetMode::Wifi:
			setIfaceLink(Iface::EthIface, false);
			setIfaceLink(Iface::WifiIface, true);
			break;
		case NetMode::EthWifi:
			setIfaceLink(Iface::EthIface, true);
			setIfaceLink(Iface::WifiIface, true);
			break;
		}
		handleWifiState();
		emit netModeChanged(mode);
	}
}


void HMISystemNetworking::cleanUp()
{
	system->cleanUp();
}


void HMISystemNetworking::setIfaceLink(Iface iface, bool up)
{
	system->setIfaceLink(scast(int, iface), up);
}


void HMISystemNetworking::setSysIfaceIpMode(Iface iface, IPMode mode)
{
	system->setSysIfaceIpMode(scast(int, iface), ipMode(mode));
}


void HMISystemNetworking::setAddress(Iface iface, const QNetworkAddressEntry &addr, QString *error)
{
	system->setAddress(scast(int, iface), addr, error);
}


void HMISystemNetworking::setRoute(Iface iface, const QHostAddress &gateway, QString *error)
{
	system->setRoute(scast(int, iface), gateway, error);
}


void HMISystemNetworking::ifaceState(Iface iface, bool &available, bool &link) const
{
	provider->ifaceState(ifaceName(iface), available, link);
}


IPMode HMISystemNetworking::ifaceSysIpMode(Iface iface) const
{
	return ipMode( system->ifaceSysIpMode(scast(int, iface)) );
}


bool HMISystemNetworking::address(Iface iface, QNetworkAddressEntry &addr) const
{
	return system->address(scast(int, iface), addr);
}


bool HMISystemNetworking::route(Iface iface, QHostAddress &gateway) const
{
	return system->route(scast(int, iface), gateway);
}


QString HMISystemNetworking::getMac(Iface iface) const
{
	return system->getMac(scast(int, iface));
}


NetMode HMISystemNetworking::netMode() const
{
	return m_netMode;
}


HMISystemNetworking *HMISystemNetworking::snw_instance = nullptr;


HMISystemNetworking *HMISystemNetworking::instance(QObject *object)
{
	if (object) {
		if (!HMISystemNetworking::snw_instance) {
			snw_instance = new HMISystemNetworking(object);
		}
		return (object == snw_instance->object)? snw_instance : nullptr;
	}
	return nullptr;
}


HMISystemNetworking::HMISystemNetworking(QObject *parent) : QObject(parent)
{
	this->object = parent;
	const auto ifaceList = getIfacesList();
	this->system = SystemNetworking::instance(this, ifaceList);
	if (platform::isArm()) {
		UdhcpProvider *dhcp = new UdhcpProvider(this->system);
		dhcp->start(IDhcpClient::Implementer); // start pipe for communication, udhcpc isn't running yet
		this->system->setDhcpProvider(dhcp);
	}

	this->provider = new NetworkingProvider(this);
	bool eth_av, eth_link, wifi_av, wifi_link;
	this->provider->ifaceState(ifaceName(Iface::EthIface), eth_av, eth_link);
	this->provider->ifaceState(ifaceName(Iface::WifiIface), wifi_av, wifi_link);
	this->m_netMode = (eth_av && wifi_av)?
			NetMode::EthWifi : (wifi_av)?
			NetMode::Wifi : NetMode::Ethernet;

	this->wifi = nullptr;

	// todo use slots?
	connect(this->system, &SystemNetworking::ifaceStateChanged, [this](int iface){
		Iface i = scast(Iface, iface);
		if (i == Iface::WifiIface) {
			this->handleWifiState();
		}
		emit this->ifaceStateChanged(i);
	});
	connect(this->system, &SystemNetworking::ifaceSysIpModeChanged, [this](int iface, NetworkingCommon::IPMode mode){
		emit this->ifaceSysIpModeChanged(scast(Iface, iface), ipMode(mode));
	});
	connect(this->system, &SystemNetworking::addressChanged, [this](int iface){
		emit this->addressChanged(scast(Iface, iface));
	});
	connect(this->system, &SystemNetworking::routeChanged, [this](int iface){
		emit this->routeChanged(scast(Iface, iface));
	});
}


HMISystemNetworking::~HMISystemNetworking()
{
	qDebug() << "~HMISystemNetworking";
	HMISystemNetworking::snw_instance = nullptr;
}


// ##################################################################################################
// ######################################### HMINetworkingProvider ##################################
// ##################################################################################################


void *HMINetworkingProvider::setAddress(void *paddr, const AddressArgs &args, QString *error)
{
	NetworkingProvider::AddressArgs provArgs;
	provArgs.addr.setIp(QHostAddress(args.ip));
	provArgs.addr.setNetmask(QHostAddress(args.mask));
	provArgs.iface = ifaceName(args.iface);
	return provider->setAddress(scast(NetworkingCommon::NwAddress*, paddr), provArgs, error);
}


void HMINetworkingProvider::delAddress(void *paddr)
{
	provider->delAddress(scast(NetworkingCommon::NwAddress*, paddr));
}


void *HMINetworkingProvider::setRoute(void *proute, const RouteArgs &args, QString *error)
{
	NetworkingProvider::RouteArgs provArgs;
	provArgs.netAddr.setIp(QHostAddress(args.netIp));
	provArgs.netAddr.setNetmask(QHostAddress(args.netMask));
	provArgs.gateway = QHostAddress(args.gateway);
	provArgs.src = QHostAddress(args.src);
	provArgs.iface = ifaceName(args.iface);
	return provider->setRoute(scast(NetworkingCommon::NwRoute*, proute), provArgs, error);
}


void HMINetworkingProvider::delRoute(void *proute)
{
	provider->delRoute(scast(NetworkingCommon::NwRoute*, proute));
}


void HMINetworkingProvider::ifaceState(Iface iface, bool &available, bool &link) const
{
	provider->ifaceState(ifaceName(iface), available, link);
}


void HMINetworkingProvider::serviceIpMode(Iface iface, IPMode &mode) const
{
	NetworkingCommon::IPMode m;
	provider->serviceIpMode(ifaceName(iface), m);
	mode = ipMode(m);
}


bool HMINetworkingProvider::systemAddress(Iface iface, QString &ip, QString &mask) const
{
	QNetworkAddressEntry addr;
	bool ret = provider->systemAddress(ifaceName(iface), addr);
	ip = addr.ip().toString();
	mask = addr.netmask().toString();
	return ret;
}


bool HMINetworkingProvider::systemRoute(Iface iface, QString &gateway) const
{
	QHostAddress addr;
	bool ret = provider->systemRoute(ifaceName(iface), addr);
	gateway = addr.toString();
	return ret;
}


NetMode HMINetworkingProvider::netMode() const
{
	if (system)
		return system->netMode();
	return NetMode::Ethernet;
}


void HMINetworkingProvider::setIfaceState(Iface iface, bool up)
{
	provider->setIfaceState( ifaceName(iface), up );
}


HMINetworkingProvider::HMINetworkingProvider(QObject *parent) : QObject(parent)
{
	system = HMISystemNetworking::snw_instance;
	provider = new NetworkingProvider(this);

	if (system) {
		connect(system, &HMISystemNetworking::netModeChanged, this, &HMINetworkingProvider::netModeChanged);
	}
	connect(provider, &NetworkingProvider::ifaceStateChanged, [this](const QString &iface){
		emit this->ifaceStateChanged( ifaceByName(iface) );
	});
	connect(provider, &NetworkingProvider::serviceIpModeChanged, [this](const QString &iface){
		emit this->serviceIpModeChanged( ifaceByName(iface) );
	});
	connect(provider, &NetworkingProvider::systemAddressChanged, [this](const QString &iface){
		emit this->systemAddressChanged( ifaceByName(iface) );
	});
	connect(provider, &NetworkingProvider::systemRouteChanged, [this](const QString &iface){
		emit this->systemRouteChanged( ifaceByName(iface) );
	});
}


HMINetworkingProvider::~HMINetworkingProvider()
{
	qDebug() << "~HMINetworkingProvider";
}

