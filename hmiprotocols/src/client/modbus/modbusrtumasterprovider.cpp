#include "modbus/modbusrtumasterprovider.h"
#include "extmodbusrtuclient.h"
#include "serialportprovider.h"
#include "typeconverter.h"
#include <QSerialPort>
#include <QDebug>
#include <QUrl>

#define scast(t,v) static_cast<t>(v)


void ModbusRtuMasterProvider::restart(const ProtocolProvider::Config *config)
{
	auto tcfg = scast(ModbusRtuMasterProvider::Config *, this->config);
	*tcfg = *(scast(const ModbusRtuMasterProvider::Config *, config));
	Config cfg = *tcfg;
	logDebug() << "restart";

    // config update end
    devDisconnect();

	// setting up default params
    ModbusMasterProvider::onRestartFillConfigCmn(scast(ModbusMasterProvider::Config *, &cfg));
	if (!cfg.dev.length()) {
		cfg.dev = SerialPortProvider::defaultDevice();
	} else {
		char number = cfg.dev.at(0).toLatin1();
		cfg.dev = (number > '0' && number <= '9')?
				SerialPortProvider::deviceByNumber(number) : cfg.dev;
	}
	if (!cfg.baudRate) {
		cfg.baudRate = 9600;
	}
	if (cfg.parity == SerialTypes::nDefParity) {
		cfg.parity = SerialTypes::NoParity;
	}
	if (!cfg.dataBits) {
		cfg.dataBits = 8;
	}
	if (cfg.stopBits == SerialTypes::nDefStopBits) {
		cfg.stopBits = SerialTypes::OneStop;
    }

	this->mbDev->setTimeout(cfg.timeout);
	this->mbDev->setNumberOfRetries(cfg.attempts);
	this->mbDev->setConnectionParameter(QModbusDevice::SerialPortNameParameter, cfg.dev);
	this->mbDev->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, cfg.baudRate);
	this->mbDev->setConnectionParameter(QModbusDevice::SerialParityParameter, TypeConverter::conv(cfg.parity));
	this->mbDev->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, cfg.dataBits);
	this->mbDev->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, TypeConverter::conv(cfg.stopBits));
    this->mbDev->setPollTo(cfg.pollTo);
    this->mbDev->setCycleTo(cfg.cycleTo);
    bool ok = devConnect();
	if (!ok) {
		logError() << "[" << cfg.addr << cfg.dev << cfg.baudRate << cfg.parity << cfg.dataBits << cfg.stopBits
				   << "] restart failed: " << strerror(errno);
	}
}


ModbusRtuMasterProvider::ModbusRtuMasterProvider(QObject *parent)
		: ModbusMasterProvider(parent)
{
	INIT_LOGGER("mmrtu");

	mbDev = new ExtModbusRtuClient(this);
    config = new ModbusRtuMasterProvider::Config();
	
	initBase();
}
