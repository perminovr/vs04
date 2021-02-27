#ifndef MODBUSSLAVEPROVIDER_H
#define MODBUSSLAVEPROVIDER_H

#include "protocolserverprovider.h"
#include "mbregister.h"
#include "hmivariant.h"

class ExtModbusServer;

class ModbusSlaveProvider : public ProtocolServerProvider
{
	Q_OBJECT
public:
    struct Config : public ProtocolProvider::Config {
		quint16 addr;   //!< default == 0
	};

	ModbusSlaveProvider(QObject *parent = nullptr);
	virtual ~ModbusSlaveProvider();

	HMIVariant::Array getData(const MbRegister &reg);

public slots:
	virtual bool pause() override;
	virtual bool resume() override;
	void updateDatabase(const MbRegister &reg, const QVariant &data);
	void insertRegister(const MbRegister &reg);

signals:
	void dataUpdated(const MbRegister &reg);

protected:
	ExtModbusServer *mbDev;
	Config *config;

	void initBase();
    void onRestartFillConfigCmn(Config *cfg);
};

#endif // MODBUSSLAVEPROVIDER_H
