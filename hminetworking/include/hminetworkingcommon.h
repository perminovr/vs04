#ifndef HMINETWORKINGCOMMON_H
#define HMINETWORKINGCOMMON_H

#include <QObject>
#include <QNetworkAddressEntry>

namespace HMINetworkingCommon
{
	Q_NAMESPACE
	/*!
	 * @enum Iface
	 * @brief Интерфейсы контроллера (2)
	 */
	enum Iface {
		ndefIface = -1,
		EthIface = 0,
		WifiIface
	};
	/*!
	 * @enum NetMode
	 * @brief Сетевой режим: текущий рабочий интерфейс
	 */
	enum NetMode {
		Ethernet,
		Wifi,
		EthWifi
	};
	/*!
	 * @enum IPMode
	 * @brief Политика IP адреса для интерфейса
	 */
	enum IPMode {
		Static,
		Dhcp
	};
	Q_ENUM_NS(HMINetworkingCommon::Iface)
	Q_ENUM_NS(HMINetworkingCommon::NetMode)
	Q_ENUM_NS(HMINetworkingCommon::IPMode)

	/*!
	 * @fn getIfacesList
	 * @brief Получение списка доступных сетевых интерфейсов устройства. 
	 * 		Реализуется на стороне главного класс программы
	 */
	const QStringList &getIfacesList();

	Iface ifaceByName(const QString &iface);
	QString ifaceName(Iface iface);

	typedef void HmiNwAddress;
	typedef void HmiNwRoute;
}

#endif // HMINETWORKINGCOMMON_H
