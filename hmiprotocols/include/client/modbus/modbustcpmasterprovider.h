#ifndef MODBUSTCPMASTERPROVIDER_H
#define MODBUSTCPMASTERPROVIDER_H

#include "modbusmasterprovider.h"

class HMINetworkingProvider;

class ModbusTcpMasterProvider : public ModbusMasterProvider
{
	Q_OBJECT
public:
	struct Config : public ModbusMasterProvider::Config {
		QString ip;	 //!< default == ""
		quint16 port;   //!< default == 0
	};

	ModbusTcpMasterProvider(QObject *parent = nullptr);
	virtual ~ModbusTcpMasterProvider() = default;

public slots:
    virtual void restart(const ProtocolProvider::Config *config) override;

private:
	HMINetworkingProvider *networking;
	int iface;
};

#endif // MODBUSTCPMASTERPROVIDER_H
