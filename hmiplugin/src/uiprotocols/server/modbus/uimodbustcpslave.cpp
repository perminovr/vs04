#include "uimodbustcpslave.h"
#include <QDebug>

#define scast(t,v) static_cast<t>(v)
#define DirectUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


void UIModbusTcpSlave::onCompleted(UINetAddress *addr)
{
	if (addr) {
        connect(addr, &UINetAddress::ifaceChanged, this, &UIModbusTcpSlave::netAddr_onPropertyChanged, DirectUniqueConnection);
        connect(addr, &UINetAddress::ipChanged, this, &UIModbusTcpSlave::netAddr_onPropertyChanged, DirectUniqueConnection);
        connect(addr, &UINetAddress::maskChanged, this, &UIModbusTcpSlave::netAddr_onPropertyChanged, DirectUniqueConnection);
        QString ip = addr->ip();
		applyAndRestart(ip);
	}
}


void UIModbusTcpSlave::applyAndRestart(const QString &ip)
{
    #if PLC_BUILD
        UIModbusSlave::restart();
		ModbusTcpSlaveProvider::Config config;
		config.addr = scast(quint16, this->m_address);
		config.ip = ip;
        config.port = scast(quint16, this->m_port);
        this->provider->restart(&config);
	#endif
}


void UIModbusTcpSlave::restart()
{
    #if PLC_BUILD
		if (m_netAddr) {
            if (m_netAddr->isComplete()) {
                onCompleted(m_netAddr);
			} else {
                connect(m_netAddr, &UINetAddress::completed, this, &UIModbusTcpSlave::onCompleted);
			}
        } else {
            applyAndRestart("");
        }
	#endif
}


UIModbusTcpSlave::UIModbusTcpSlave(QObject *parent) : UIModbusSlave(parent)
{
	IF_PLC_BUILD(provider = new ModbusTcpSlaveProvider(this));
	// UIProtocolCtlCfgClass_construct();
	m_netAddr = nullptr;
	m_port = 0;
	initBase();
}


UIModbusTcpSlave::~UIModbusTcpSlave()
{
	qDebug() << "~UIModbusTcpSlave";
}


void UIModbusTcpSlave::setNetAddr(UINetAddress *netAddr)
{
    m_netAddr = netAddr;
    if (isStarted()) restart();
	emit netAddrChanged();
}


void UIModbusTcpSlave::setPort(int port)
{
    m_port = scast(quint16, port);
    if (isStarted()) restart();
	emit portChanged();
}


UINetAddress *UIModbusTcpSlave::netAddr()
{
	return m_netAddr;
}


int UIModbusTcpSlave::port()
{
	return m_port;
}


void UIModbusTcpSlave::netAddr_onPropertyChanged()
{
    if (isStarted()) restart();
	emit netAddrChanged();
}


// #define baseFromArray(var, arrmemb, tpsz) (decltype(var)) ( (tpsz) (*((const char*)(&(arrmemb)))) )
// #define getBaseFromArray(var, arrmemb, tpsz) var=baseFromArray(var, arrmemb, tpsz)
// #define getBaseFromArrayAndShift(var, arr, it, tpsz) { getBaseFromArray(var, arr[it], tpsz); it+=sizeof(tpsz); }
// #define strFromArray(arr) ((const char *)arr)
// #define getStringFromArrayAndShift(str, arr, it) { const char *tstr = strFromArray(arr); it += strlen(tstr) + 1; str = tstr; }

// /*
// address
// netAddr
// 	ip
// 	mask
// 	iface
// port
// */

// QByteArray UIProtocolCtlCfgClassName::toBytes() const
// {
// 	UIModbusTcpSlave *self = UIProtocolCtlCfgClass_getUIProtocol();
// 	QByteArray ret;
// 	ret.append((const char *)(&self->m_address), 1);
// 	ret.append(self->m_netAddr->ip());
// 	ret.push_back((char)0);
// 	ret.append(self->m_netAddr->mask());
// 	ret.push_back((char)0);
// 	UINetAddress::Interface iface = self->m_netAddr->iface();
// 	ret.append((const char *)(&iface), 1);
// 	ret.append((const char *)(&self->m_port), 2);
// 	return ret;
// }


// int UIProtocolCtlCfgClassName::fromBytes(const quint8 *array)
// {
// 	int iprm;
// 	QString sprm;
// 	UIModbusTcpSlave *self = UIProtocolCtlCfgClass_getUIProtocol();
// 	int it = 0;
// 	getBaseFromArrayAndShift(iprm, array, it, quint8); self->setAddress(iprm);
// 	getStringFromArrayAndShift(sprm, array, it); self->m_netAddr->setIp(sprm);
// 	getStringFromArrayAndShift(sprm, array, it); self->m_netAddr->setMask(sprm);
// 	getBaseFromArrayAndShift(iprm, array, it, quint8); self->m_netAddr->setIface(scast(UINetAddress::Interface, iprm));
// 	getBaseFromArrayAndShift(iprm, array, it, quint16); self->setPort(iprm);
//	emit this->changed();
// 	return it;
// }
