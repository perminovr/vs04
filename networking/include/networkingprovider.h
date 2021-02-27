#ifndef NETWORKINGPROVIDER_H
#define NETWORKINGPROVIDER_H

#include "networkingcommon.h"
#include <QNetworkAddressEntry>

class SystemNetworking;
class Networking;

/*!
 * @class NetworkingProvider
 * @brief Предоставляет доступ к сетевой компоненте контроллера
 * @details Предоставляет доступ к установке/изменению/удалению адресов и маршрутов
 * контроллера.
 * Предоставляет защищенный доступ к системным функциям: изменение/чтение состояния
 * интерфейсов, чтение политики IP адресации инфтерфейса, чтение системных адресов
 * и маршрутов.
 */
class NetworkingProvider : public QObject
{
	Q_OBJECT
public:
	struct AddressArgs {
		QNetworkAddressEntry addr;
		QString iface;
	};
	NetworkingCommon::NwAddress *setAddress(NetworkingCommon::NwAddress *, const AddressArgs &args, QString *error = nullptr);
	void delAddress(NetworkingCommon::NwAddress *);
	struct RouteArgs {
		QNetworkAddressEntry netAddr;
		QHostAddress gateway;
		QHostAddress src;
		QString iface;
	};
	NetworkingCommon::NwRoute *setRoute(NetworkingCommon::NwRoute *, const RouteArgs &args, QString *error = nullptr);
	void delRoute(NetworkingCommon::NwRoute *);

	void ifaceState(const QString &iface, bool &available, bool &link) const;
	void serviceIpMode(const QString &iface, NetworkingCommon::IPMode &mode) const;
	bool systemAddress(const QString &iface, QNetworkAddressEntry &addr) const;
	bool systemRoute(const QString &iface, QHostAddress &gateway) const;

	NetworkingProvider(QObject *parent);
	virtual ~NetworkingProvider();

public slots:
	void setIfaceState(const QString &iface, bool up);

signals:
	void ifaceStateChanged(const QString &iface);
	void serviceIpModeChanged(const QString &iface);
	void systemAddressChanged(const QString &iface);
	void systemRouteChanged(const QString &iface);

private slots:
	void onAddressChanged(int iface);
	void onRouteChanged(int iface);
	void onIfaceStateChanged(int iface);
	void onIfaceSysIpModeChanged(int iface, NetworkingCommon::IPMode mode);

private:
	Networking *networking;
	SystemNetworking *system;
};

#endif // NETWORKINGPROVIDER_H
