#ifndef MODBUSRTUSLAVEPROVIDER_H
#define MODBUSRTUSLAVEPROVIDER_H

#include "serialtypes.h"
#include "modbusslaveprovider.h"

class ModbusRtuSlaveProvider : public ModbusSlaveProvider
{
	Q_OBJECT
public:
	struct Config : public ModbusSlaveProvider::Config {
		QString dev;		//!< default == ""
		int baudRate;	   //!< default == 0
		SerialTypes::Parity parity;	  //!< default == nDefParity
		int dataBits;	   //!< default == 0
		SerialTypes::StopBits stopBits;  //!< default == nDefStopBits
	};

	struct ClientAddress : public ProtocolServerProvider::ClientAddress {
	};

	ModbusRtuSlaveProvider(QObject *parent = nullptr);
	virtual ~ModbusRtuSlaveProvider() = default;

public slots:
    virtual void restart(const ProtocolProvider::Config *config) override;
};

#endif // MODBUSRTUSLAVEPROVIDER_H
