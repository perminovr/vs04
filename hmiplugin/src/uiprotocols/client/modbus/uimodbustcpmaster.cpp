#include "uimodbustcpmaster.h"
#include "uitypeconverter.h"
#include <QDebug>

#define scast(t,v) static_cast<t>(v)
#define DirectUniqueConnection \
    scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


void UIModbusTcpMaster::onCompleted(UINetAddress *src)
{
    if (src) {
        connect(src, &UINetAddress::ifaceChanged, this, &UIModbusTcpMaster::srcAddr_onPropertyChanged, DirectUniqueConnection);
        connect(src, &UINetAddress::ipChanged, this, &UIModbusTcpMaster::srcAddr_onPropertyChanged, DirectUniqueConnection);
        connect(src, &UINetAddress::maskChanged, this, &UIModbusTcpMaster::srcAddr_onPropertyChanged, DirectUniqueConnection);
        applyAndRestart();
    }
}


void UIModbusTcpMaster::applyAndRestart()
{
    #if PLC_BUILD
        UIModbusMaster::restart();
        ModbusTcpMasterProvider::Config config;
        config.addr = scast(quint16, m_address);
        config.ip = m_ip;
        config.port = scast(quint16, m_port);
        config.timeout = scast(quint16, m_timeOut);
        config.attempts = scast(quint16, m_attempts);
        config.pollTo = scast(quint16, m_pollTo);
        config.cycleTo = scast(quint16, m_cycleTo);
        this->provider->restart(&config);
    #endif
}


void UIModbusTcpMaster::restart()
{
    #if PLC_BUILD
        if (m_srcAddr) {
            if (m_srcAddr->isComplete()) {
                onCompleted(m_srcAddr);
            } else {
                connect(m_srcAddr, &UINetAddress::completed, this, &UIModbusTcpMaster::onCompleted);
            }
        } else {
            applyAndRestart();
        }
    #endif
}


UIModbusTcpMaster::UIModbusTcpMaster(QObject *parent) : UIModbusMaster(parent)
{
	IF_PLC_BUILD(provider = new ModbusTcpMasterProvider(this));
    m_srcAddr = nullptr;
	m_port = 0;
    initBase();
}


UIModbusTcpMaster::~UIModbusTcpMaster()
{
	qDebug() << "~UIModbusTcpMaster";
}


void UIModbusTcpMaster::setIp(const QString &ip)
{
    m_ip = ip;
    if (isStarted()) restart();
	emit ipChanged();
}


void UIModbusTcpMaster::setPort(int port)
{
    m_port = scast(quint16, port);
    if (isStarted()) restart();
	emit portChanged();
}


void UIModbusTcpMaster::setSrcAddr(UINetAddress *srcAddr)
{
    m_srcAddr = srcAddr;
    if (isStarted()) restart();
    emit srcAddrChanged();
}


QString UIModbusTcpMaster::ip()
{
	return m_ip;
}


int UIModbusTcpMaster::port()
{
	return m_port;
}


UINetAddress *UIModbusTcpMaster::srcAddr()
{
    return m_srcAddr;
}


void UIModbusTcpMaster::srcAddr_onPropertyChanged()
{
    if (isStarted()) restart();
    emit srcAddrChanged();
}

