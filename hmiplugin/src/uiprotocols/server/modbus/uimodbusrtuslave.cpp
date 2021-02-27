#include "uimodbusrtuslave.h"
#include "uitypeconverter.h"

#define scast(t,v) static_cast<t>(v)
#define DirectUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


#if PLC_BUILD
static void setConfFromUISerial(ModbusRtuSlaveProvider::Config &cfg, UISerialPort *port)
{
	cfg.dev = port->dev();
	cfg.baudRate = port->baudRate();
	cfg.parity = UITypeConverter::conv(scast(UISerialTypes::Parity, port->parity()));
	cfg.dataBits = port->dataBits();
	cfg.stopBits = UITypeConverter::conv(scast(UISerialTypes::StopBits, port->stopBits()));
}


void UIModbusRtuSlave::applyAndRestart(ModbusRtuSlaveProvider::Config &cfg)
{
    UIModbusSlave::restart();
    cfg.addr = scast(quint16, this->m_address);
    provider->restart(&cfg);
}
#endif


void UIModbusRtuSlave::onCompleted(UISerialPort *port)
{
	#if PLC_BUILD
		if (port) {
			connect(port, &UISerialPort::devChanged, this, &UIModbusRtuSlave::port_onPropertyChanged, DirectUniqueConnection);
			connect(port, &UISerialPort::baudRateChanged, this, &UIModbusRtuSlave::port_onPropertyChanged, DirectUniqueConnection);
			connect(port, &UISerialPort::parityChanged, this, &UIModbusRtuSlave::port_onPropertyChanged, DirectUniqueConnection);
			connect(port, &UISerialPort::dataBitsChanged, this, &UIModbusRtuSlave::port_onPropertyChanged, DirectUniqueConnection);
			connect(port, &UISerialPort::stopBitsChanged, this, &UIModbusRtuSlave::port_onPropertyChanged, DirectUniqueConnection);
			ModbusRtuSlaveProvider::Config config;
			setConfFromUISerial(config, port);
			applyAndRestart(config);
		}
	#endif
}


void UIModbusRtuSlave::restart()
{
	#if PLC_BUILD
		ModbusRtuSlaveProvider::Config config;
		config.dev = "";
		config.baudRate = 0;
		config.parity = SerialTypes::nDefParity;
		config.dataBits = 0;
		config.stopBits = SerialTypes::nDefStopBits;
		if (m_port) {
			if (m_port->isComplete()) {
				setConfFromUISerial(config, m_port);
			} else {
				connect(m_port, &UISerialPort::completed, this, &UIModbusRtuSlave::onCompleted);
				return;
			}
		}
		applyAndRestart(config);
	#endif
}


UIModbusRtuSlave::UIModbusRtuSlave(QObject *parent) : UIModbusSlave(parent)
{
	IF_PLC_BUILD(provider = new ModbusRtuSlaveProvider(this));
	m_port = nullptr;
	initBase();
}


UIModbusRtuSlave::~UIModbusRtuSlave()
{
	qDebug() << "~UIModbusRtuSlave";
}


void UIModbusRtuSlave::setPort(UISerialPort *port)
{
    m_port = port;
    if (isStarted()) restart();
	emit portChanged();
}


UISerialPort *UIModbusRtuSlave::port()
{
	return m_port;
}


void UIModbusRtuSlave::port_onPropertyChanged()
{
    if (isStarted()) restart();
	emit portChanged();
}
