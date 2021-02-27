#include "idhcpclient.h"

/*!
 * @class DhcpNotifier
 * @brief Предназначен для уведомления основной программы о dhcp-событиях через pipe
*/
class DhcpNotifier : public IDhcpClient {
    Q_OBJECT
public:
	virtual bool runDhcpDaemon(const QString &iface) override { Q_UNUSED(iface); return true; }
	virtual bool stopDhcpDaemon(const QString &iface) override { Q_UNUSED(iface); return true; }
    DhcpNotifier(QObject *parent = nullptr) : IDhcpClient(parent) {}
    virtual ~DhcpNotifier() {}
};
