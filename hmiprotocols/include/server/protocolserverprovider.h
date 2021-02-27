#ifndef PROTOCOLSERVERPROVIDER_H
#define PROTOCOLSERVERPROVIDER_H

#include <QTimer>
#include "protocolprovider.h"

class ProtocolServerProvider : public ProtocolProvider
{
    Q_OBJECT
public:

	struct ClientAddress {
		// protocol-specific address
	};

    ProtocolServerProvider(QObject *parent = nullptr) : ProtocolProvider(parent) { }
    virtual ~ProtocolServerProvider() = default;

signals:
    void clientConnected(const ClientAddress *address);
    void clientDisconnected(const ClientAddress *address);

protected:

private:
};

#endif // PROTOCOLSERVERPROVIDER_H
