#ifndef NETLINKWORKER_H
#define NETLINKWORKER_H

#include <QObject>
#include "networking.h"
#include "nlwrapper.h"

#include <stdio.h>
#include <functional>

/*!
 * @class NetLinkWorker
 * @brief Ожидание сетевых событий: изменение интерфейсов, адресов,
 * маршрутов.
*/
class NetLinkWorker : public QObject
{
	Q_OBJECT

public:
	NetLinkWorker(Networking *net, QObject *parent = nullptr);
	virtual ~NetLinkWorker();

	static int parseEvent(nl_msg_t msg, void *arg);

public slots:
	void doWork();
	void stopWork();

signals:
	void nlLinkEvent(const QString &ifaceName, bool up);
	void nlAddrEvent(const QString &ifaceName, const QNetworkAddressEntry &addr, bool add, quint8 flags);
	void nlRouteEvent(const QString &ifaceName, const QNetworkAddressEntry &addr, const QString &gateway,
			bool dhcp, bool add);
	void done();

protected:
	NlEventSock *evSk;
	NlEventSock &getSock();
	Networking *net;
};

#endif // NETLINKWORKER_H
