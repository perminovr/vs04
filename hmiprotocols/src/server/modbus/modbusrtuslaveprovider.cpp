#include "modbus/modbusrtuslaveprovider.h"
#include "extmodbusrtuserver.h"
#include "serialportprovider.h"
#include "typeconverter.h"
#include <QSerialPort>
#include <QDebug>
#include <QUrl>

#define scast(t,v) static_cast<t>(v)


void ModbusRtuSlaveProvider::restart(const ProtocolProvider::Config *config)
{
	auto tcfg = scast(ModbusRtuSlaveProvider::Config *, this->config);
	*tcfg = *(scast(const ModbusRtuSlaveProvider::Config *, config));
	Config cfg = *tcfg;
	logDebug() << "restart";

	// config update end
	this->mbDev->disconnectDevice();

    // setting up default params
    ModbusSlaveProvider::onRestartFillConfigCmn(scast(ModbusSlaveProvider::Config *, &cfg));
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

	this->mbDev->setValue(QModbusServer::DeviceBusy, 0);
	this->mbDev->setConnectionParameter(QModbusDevice::SerialPortNameParameter, cfg.dev);
	this->mbDev->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, cfg.baudRate);
	this->mbDev->setConnectionParameter(QModbusDevice::SerialParityParameter, TypeConverter::conv(cfg.parity));
	this->mbDev->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, cfg.dataBits);
	this->mbDev->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, TypeConverter::conv(cfg.stopBits));
	this->mbDev->setServerAddress(cfg.addr);
	bool ok = this->mbDev->connectDevice();
	if (!ok) {
		logError() << "[" << cfg.addr << cfg.dev << cfg.baudRate << cfg.parity << cfg.dataBits << cfg.stopBits
				   << "] restart failed: " << strerror(errno);
	}
}


ModbusRtuSlaveProvider::ModbusRtuSlaveProvider(QObject *parent) : ModbusSlaveProvider(parent)
{
	INIT_LOGGER("msrtu");

	mbDev = new ExtModbusRtuServer(this);
	config = new ModbusRtuSlaveProvider::Config();

	initBase();
}
