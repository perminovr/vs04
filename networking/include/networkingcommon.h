#ifndef NETWORKINGCOMMON_H
#define NETWORKINGCOMMON_H

#include <QObject>
#include <QNetworkAddressEntry>

namespace NetworkingCommon 
{
	Q_NAMESPACE
	/*!
	 * @enum IPMode
	 * @brief Политика IP адреса для интерфейса
	 */
	enum IPMode {
		ipmnDef,
		Static,
		Dhcp
	};
	Q_ENUM_NS(NetworkingCommon::IPMode)

	/*!
	 * @class NwAddress
	 * @brief Net Work Address - хранилище системной информации о IP адресе
	 */
	class NwAddress;
	/*!
	 * @class NwRoute
	 * @brief Net Work Route - хранилище системной информации о маршруте
	 */
	class NwRoute;

	bool operator==(const NwAddress &k1, const NwAddress &k2);
	uint qHash(const NwAddress &key, uint seed = 0);
	bool operator==(const NwRoute &k1, const NwRoute &k2);
	uint qHash(const NwRoute &key, uint seed = 0);
}

#endif // NETWORKINGCOMMON_H
