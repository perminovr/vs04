#ifndef MODBUSRTUMASTERPROVIDER_H
#define MODBUSRTUMASTERPROVIDER_H

#include "serialtypes.h"
#include "modbusmasterprovider.h"

class ModbusRtuMasterProvider : public ModbusMasterProvider
{
	Q_OBJECT
public:
	struct Config : public ModbusMasterProvider::Config {
		QString dev;		//!< default == ""
		int baudRate;	   //!< default == 0
		SerialTypes::Parity parity;	  //!< default == nDefParity
		int dataBits;	   //!< default == 0
		SerialTypes::StopBits stopBits;  //!< default == nDefStopBits
	};

	ModbusRtuMasterProvider(QObject *parent = nullptr);
	virtual ~ModbusRtuMasterProvider() = default;

public slots:
    virtual void restart(const ProtocolProvider::Config *config) override;
};

#endif // MODBUSRTUMASTERPROVIDER_H
