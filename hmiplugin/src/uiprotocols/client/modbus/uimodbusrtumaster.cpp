#include "uimodbusrtumaster.h"
#include "uitypeconverter.h"

#define scast(t,v) static_cast<t>(v)
#define DirectUniqueConnection \
    scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


#if PLC_BUILD
static void setConfFromUISerial(ModbusRtuMasterProvider::Config &cfg, UISerialPort *port)
{
	cfg.dev = port->dev();
	cfg.baudRate = port->baudRate();
	cfg.parity = UITypeConverter::conv(scast(UISerialTypes::Parity, port->parity()));
	cfg.dataBits = port->dataBits();
	cfg.stopBits = UITypeConverter::conv(scast(UISerialTypes::StopBits, port->stopBits()));
}


void UIModbusRtuMaster::applyAndRestart(ModbusRtuMasterProvider::Config &cfg)
{
	UIModbusMaster::restart();
    cfg.addr = scast(quint16, this->m_address);
    provider->restart(&cfg);
}
#endif


void UIModbusRtuMaster::onCompleted(UISerialPort *port)
{
	#if PLC_BUILD
        if (port) {
            connect(port, &UISerialPort::devChanged, this, &UIModbusRtuMaster::port_onPropertyChanged, DirectUniqueConnection);
            connect(port, &UISerialPort::baudRateChanged, this, &UIModbusRtuMaster::port_onPropertyChanged, DirectUniqueConnection);
            connect(port, &UISerialPort::parityChanged, this, &UIModbusRtuMaster::port_onPropertyChanged, DirectUniqueConnection);
            connect(port, &UISerialPort::dataBitsChanged, this, &UIModbusRtuMaster::port_onPropertyChanged, DirectUniqueConnection);
            connect(port, &UISerialPort::stopBitsChanged, this, &UIModbusRtuMaster::port_onPropertyChanged, DirectUniqueConnection);
            ModbusRtuMasterProvider::Config config;
			setConfFromUISerial(config, port);
			applyAndRestart(config);
		}
	#endif
}


void UIModbusRtuMaster::restart()
{
	#if PLC_BUILD
		ModbusRtuMasterProvider::Config config;
		config.dev = "";
		config.baudRate = 0;
		config.parity = SerialTypes::nDefParity;
		config.dataBits = 0;
		config.stopBits = SerialTypes::nDefStopBits;
		if (m_port) {
			if (m_port->isComplete()) {
				setConfFromUISerial(config, m_port);
			} else {
				connect(m_port, &UISerialPort::completed, this, &UIModbusRtuMaster::onCompleted);
				return;
			}
		}
		applyAndRestart(config);
	#endif
}


UIModbusRtuMaster::UIModbusRtuMaster(QObject *parent) : UIModbusMaster(parent)
{
	IF_PLC_BUILD(provider = new ModbusRtuMasterProvider(this));
	m_port = nullptr;
	initBase();
}


UIModbusRtuMaster::~UIModbusRtuMaster()
{
	qDebug() << "~UIModbusRtuMaster";
}


void UIModbusRtuMaster::setPort(UISerialPort *port)
{
    m_port = port;
    if (isStarted()) restart();
    emit portChanged();
}


UISerialPort *UIModbusRtuMaster::port()
{
	return m_port;
}


void UIModbusRtuMaster::port_onPropertyChanged()
{
    if (isStarted()) restart();
    emit portChanged();
}
