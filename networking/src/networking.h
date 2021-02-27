#ifndef NETWORKING_H
#define NETWORKING_H

#include "networkingcommon.h"
#include <QHash>
#include <QString>

class NlSock;
class NetLinkWorker;
class Iface;

using namespace NetworkingCommon;

/*!
 * @class Networking
 * @brief Сетевая компонента контроллера
 * @details Сетевая компонента контроллера (далее - компонента). Работает через
 * Linux NetLink
 *
 * Предоставляет добавление, удаление ip и route (далее - netobject) в систему.
 *
 * Методы netobject
 *	  create* - создание объекта
 *	  set* - установка параметров объекта
 *	  unset* - деинициализация параметров объекта
 *	  del* - удаление объекта (с выполенением деинициализации)
 *
 * Предоставляет включение/отключение сетевых интерфейсов контроллера и
 * информацию по их состоянию.
 *
 * Предоставляет управление системными интерфейсами и события по изменению их
 * состояния.
*/
class Networking : public QObject
{
	Q_OBJECT
public:
	static Networking *instance(const QStringList &ifaces = {}, QObject *parent = nullptr);

	QString ifaceName(int iface) const;
	int ifaceByName(const QString &iface) const;
	int ifacesSize() const;

	NwAddress *createAddress(int iface) const;
	struct AddressArgs {
		QNetworkAddressEntry newAddr;
		int newIface;
		AddressArgs() {
			newIface = -1;
		}
	};
	int setAddress(NwAddress *, const AddressArgs &args, bool forceRefresh = false);
	int unsetAddress(NwAddress *, bool forceDelete = false);
	void delAddress(NwAddress *);

	NwRoute *createRoute(int iface, const QNetworkAddressEntry &netAddr)  const;
	struct RouteArgs {
		QHostAddress newGw;
		QHostAddress newSrc;
		int newIface;
		RouteArgs() {
			newIface = -1;
		}
	};
	int setRoute(NwRoute *, const RouteArgs &args);
	int unsetRoute(NwRoute *);
	void delRoute(NwRoute *);

	int setIfaceLink(int iface, bool up);
	void ifaceStat(int iface, bool &available, bool &link) const;

	QString getMac(int iface);

	void cleanUp();

public slots:

signals:
	void ifaceStateChanged(int iface);

private slots:
	void onNlLinkEvent(const QString &ifaceName, bool up);

private:
	Networking(const QStringList &ifaces, QObject *parent = nullptr);
	virtual ~Networking();
	Q_DISABLE_COPY(Networking)

	static Networking *nw_instance;

	friend class NetLinkWorker;
	NetLinkWorker *worker;

	NlSock *sk;

	int addAddress(NwAddress *, const AddressArgs &args);
	int rmAddress(NwAddress *, bool forceDelete = false);
	int addRoute(NwRoute *, const RouteArgs &args);
	int rmRoute(NwRoute *);

	void p_ifaceStat(int iface, bool &available, bool &link);

	NlSock &getSock();

	QList <Iface> ifaces;
	QHash <NwAddress, int> addrUsers;

	mutable QHash <NwAddress*, bool> addrPtrs;
	mutable QHash <NwRoute*, bool> routePtrs;
};

#endif // NETWORKING_H
