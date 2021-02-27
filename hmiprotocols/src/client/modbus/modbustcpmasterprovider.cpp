#include "modbus/modbustcpmasterprovider.h"
#include "modbus/extmodbustcpclient.h"
#include "hminetworkingprovider.h"
#include <QDebug>
#include <QUrl>
#include <QModbusTcpClient>

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


void ModbusTcpMasterProvider::restart(const ProtocolProvider::Config *config)
{
	auto tcfg = scast(ModbusTcpMasterProvider::Config *, this->config);
	*tcfg = *(scast(const ModbusTcpMasterProvider::Config *, config));
	Config cfg = *tcfg;
	logDebug() << "restart";

	// config update end
    devDisconnect();

	int sysIface;
	QString sysip;
	getSysNetInfo(networking, sysip, sysIface);

	// setting up default params
    ModbusMasterProvider::onRestartFillConfigCmn(scast(ModbusMasterProvider::Config *, &cfg));
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

	this->mbDev->setTimeout(cfg.timeout);
	this->mbDev->setNumberOfRetries(cfg.attempts);
	this->mbDev->setConnectionParameter(QModbusDevice::NetworkAddressParameter, cfg.ip);
	this->mbDev->setConnectionParameter(QModbusDevice::NetworkPortParameter, scast(int, cfg.port));
    this->mbDev->setPollTo(cfg.pollTo);
    this->mbDev->setCycleTo(cfg.cycleTo);
    bool ok = devConnect();
	if (!ok) {
		logError() << "[" << cfg.addr << cfg.ip << cfg.port << "] restart failed:" << strerror(errno);
	}
}


ModbusTcpMasterProvider::ModbusTcpMasterProvider(QObject *parent)
		: ModbusMasterProvider(parent)
{
	INIT_LOGGER("mmtcp");

	mbDev = new ExtModbusTcpClient(this);
    config = new ModbusTcpMasterProvider::Config();

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
