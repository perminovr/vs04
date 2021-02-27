#ifndef MODBUSMASTERPROVIDER_H
#define MODBUSMASTERPROVIDER_H

#include "protocolclientprovider.h"
#include "mbregister.h"
#include "hmivariant.h"

class ExtModbusClient;

class ModbusMasterProvider : public ProtocolClientProvider
{
	Q_OBJECT
public:
    struct Config : public ProtocolProvider::Config {
        quint16 addr;       //!< default == 0
		quint16 timeout;	//!< default == 5000
		quint16 attempts;   //!< default == 3
        quint16 pollTo;     //!< default == 100
        quint16 cycleTo;	//!< default == 1000
	};

	ModbusMasterProvider(QObject *parent = nullptr);
    virtual ~ModbusMasterProvider();

public slots:
	virtual bool pause() override;
	virtual bool resume() override;
	void updateDatabase(const MbRegister &reg, const QVariant &data);
	void setPolledRegister(const MbRegister &reg, bool rdpolled, bool wrpolled = false);
    void forceWriteAllDatabase();

signals:
    void connectedChange(bool state);
	void dataUpdated(const MbRegister &reg, const HMIVariant::Array &data);

protected:
	ExtModbusClient *mbDev;
	Config *config;

	void initBase();
    void onRestartFillConfigCmn(Config *cfg);

    bool devConnect();
    void devDisconnect();
};

#endif // MODBUSMASTERPROVIDER_H
