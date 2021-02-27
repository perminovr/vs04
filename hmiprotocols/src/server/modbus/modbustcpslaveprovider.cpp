#include "modbus/modbustcpslaveprovider.h"
#include "extmodbustcpserver.h"
#include "hminetworkingprovider.h"
#include "typeconverter.h"
#include <QDebug>
#include <QUrl>

#define scast(t,v) static_cast<t>(v)


static void getSysNetInfo(HMINetworkingProvider *provider, QString &ip, int &iface)
{
	QString mask;
	switch (provider->netMode()) {
	case NetMode::Wifi:
		iface = scast(int, HMINetworkingCommon::Iface::WifiIface);
		provider->systemAddress(HMINetworkingCommon::Iface::WifiIface, ip, mask);
		break;
	default:
		iface = scast(int, HMINetworkingCommon::Iface::EthIface);
		provider->systemAddress(HMINetworkingCommon::Iface::EthIface, ip, mask);
		break;
	}
}


void ModbusTcpSlaveProvider::restart(const ProtocolProvider::Config *config)
{
	auto tcfg = scast(ModbusTcpSlaveProvider::Config *, this->config);
	*tcfg = *(scast(const ModbusTcpSlaveProvider::Config *, config));
	Config cfg = *tcfg;
	logDebug() << "restart";

	// config update end
	this->mbDev->disconnectDevice();

	int sysIface;
	QString sysip;
	getSysNetInfo(networking, sysip, sysIface);

    // setting up default params
    ModbusSlaveProvider::onRestartFillConfigCmn(scast(ModbusSlaveProvider::Config *, &cfg));
	if (!cfg.ip.length()) {
		this->m_usingSysComSlot = true;
		this->iface = sysIface;
		cfg.ip = sysip;
	} else {
		this->m_usingSysComSlot = false;
	}
	if (!cfg.port) {
		cfg.port = 502;
	}

	this->mbDev->setValue(QModbusServer::DeviceBusy, 0);
	this->mbDev->setConnectionParameter(QModbusDevice::NetworkAddressParameter, cfg.ip);
	this->mbDev->setConnectionParameter(QModbusDevice::NetworkPortParameter, scast(int, cfg.port));
	this->mbDev->setServerAddress(cfg.addr);
	bool ok = this->mbDev->connectDevice();
	if (!ok) {
		logError() << "[" << cfg.addr << cfg.ip << cfg.port << "] restart failed:" << strerror(errno);
	}
}


ModbusTcpSlaveProvider::ModbusTcpSlaveProvider(QObject *parent) : ModbusSlaveProvider(parent)
{
	INIT_LOGGER("mstcp");

	mbDev = new ExtModbusTcpServer(this);
	config = new ModbusTcpSlaveProvider::Config();

	networking = new HMINetworkingProvider(this);
	connect(networking, &HMINetworkingProvider::netModeChanged, [this](NetMode mode){
		Q_UNUSED(mode)
		if (this->m_usingSysComSlot) {
			this->restart(this->config);
		}
	});
	connect(networking, &HMINetworkingProvider::systemAddressChanged, [this](Iface iface){
		if (scast(int, iface) == this->iface) {
			if (this->m_usingSysComSlot) {
				this->restart(this->config);
			}
		}
	});

	this->iface = -1;

	initBase();
}
