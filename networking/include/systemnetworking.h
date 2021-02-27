#ifndef SYSTEMNETWORKING_H
#define SYSTEMNETWORKING_H

#include "networkingcommon.h"
#include <QVector>

class Networking;
class NetworkingProvider;
class IDhcpClient;

class NwObject;
class IfaceStat;

/*!
 * @class SystemNetworking
 * @brief Системная сетевая компонента
 * @details Прямой доступ к функциям компоненты может осуществлять
 * только системный объект, а также friend class @ref NetworkingProvider.
 * Системным считается первый объект, получивший @ref SystemNetworking::instance.
 * Прочие объекты могут получать защищенный доступ к функциям через
 * @ref NetworkingProvider.
 *
 * Предоставляет включение/отключение сетевых интерфейсов контроллера и
 * информацию по их состоянию.
 *
 * Предоставляет доступ к dhcp client daemon. Запускает и останавливает
 * демона по внешней команде. Вся работа по добавлению/удалению IP/Route
 * проводится внутри компоненты: демон при получении/освобождении
 * адресов и маршрутов сообщает компоненте через @ref SystemNetworking::onDhcpProviderNotify
 * параметры события. Исключение: dns и domain должен устанавливать
 * рабочий скрипт демона.
 *
 * Системным адресом/маршрутом считается только один IP/Route. Компонента
 * полностью отвечает за значение их параметров: устанавливает/удаляет
 * IP/Route по изменению от dhcp или от пользователя (static). Любой, кто
 * использует системный адрес/маршрут, должен по получению соответствующего
 * сигнала об изменениях установить собственные параметры работы:
 * @ref SystemNetworking::addressChanged : @ref SystemNetworking::address
 * @ref SystemNetworking::routeChanged : @ref SystemNetworking::route
 */
class SystemNetworking : public QObject
{
	Q_OBJECT
public:
	static SystemNetworking *instance(QObject *object, const QStringList &ifaces = {});

	NetworkingCommon::IPMode ifaceSysIpMode(int iface) const;

	bool address(int iface, QNetworkAddressEntry &addr) const;
	bool route(int iface, QHostAddress &gateway) const;

	bool setDhcpProvider(IDhcpClient *);

	QString getMac(int iface) const;

	void cleanUp();

public slots:
	void setIfaceLink(int iface, bool up);
	void setSysIfaceIpMode(int iface, NetworkingCommon::IPMode mode);

	void setAddress(int iface, const QNetworkAddressEntry &addr, QString *error = nullptr);
	void setRoute(int iface, const QHostAddress &gateway, QString *error = nullptr);

signals:
	void ifaceStateChanged(int iface);
	void ifaceSysIpModeChanged(int iface, NetworkingCommon::IPMode mode);

	void addressChanged(int iface);
	void routeChanged(int iface);

private slots:
	void onIfaceStateChanged(int iface);
	void onDhcpProviderNotify(const QString &ifaceName, const QNetworkAddressEntry &addr, const QHostAddress &defGateway, bool add);

private:
	SystemNetworking(const QStringList &ifaces, QObject *parent);
	virtual ~SystemNetworking();
	Q_DISABLE_COPY(SystemNetworking)

	friend class NetworkingProvider;

	static SystemNetworking *snw_instance;

	mutable Networking *networking;
	const QObject *sysObject;

	IDhcpClient *dhcp;
	QVector<NwObject> curNwObjs;
	QVector<NwObject> stcNwObjs;

	void p_sysIfaceIpMode(int iface, NetworkingCommon::IPMode mode, bool set);
};

#endif // SYSTEMNETWORKING_H
