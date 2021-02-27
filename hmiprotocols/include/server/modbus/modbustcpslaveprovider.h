#ifndef MODBUSTCPSLAVEPROVIDER_H
#define MODBUSTCPSLAVEPROVIDER_H

#include "modbusslaveprovider.h"

class HMINetworkingProvider;

class ModbusTcpSlaveProvider : public ModbusSlaveProvider
{
	Q_OBJECT
public:
	struct Config : public ModbusSlaveProvider::Config {
		QString ip;		//!< default == ""
		quint16 port;	//!< default == 0
	};

	struct ClientAddress : public ProtocolServerProvider::ClientAddress {
		QString ip;
		quint16 port;
	};

	ModbusTcpSlaveProvider(QObject *parent = nullptr);
	virtual ~ModbusTcpSlaveProvider() = default;

public slots:
    virtual void restart(const ProtocolProvider::Config *config) override;

private:
	HMINetworkingProvider *networking;
	int iface;
};

#endif // MODBUSTCPSLAVEPROVIDER_H
