#include "uinetroute.h"
#include "uitypeconverter.h"

#define scast(t,v) static_cast<t>(v)
#define DirectUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


void UINetRoute::doSetup()
{
	#if PLC_BUILD
		QString err;
		HMINetworkingProvider::RouteArgs args;
		if (m_src) {
			connect(m_src, &UINetAddress::ifaceChanged, this, &UINetRoute::netAddr_onPropertyChanged, DirectUniqueConnection);
			connect(m_src, &UINetAddress::ipChanged, this, &UINetRoute::netAddr_onPropertyChanged, DirectUniqueConnection);
			connect(m_src, &UINetAddress::maskChanged, this, &UINetRoute::netAddr_onPropertyChanged, DirectUniqueConnection);
			this->m_iface = this->m_src->iface();
			args.src = this->m_src->ip();
		}
		args.iface = UITypeConverter::conv(this->m_iface);
		args.netIp = this->m_ip;
		args.netMask = this->m_mask;
		args.gateway = this->m_gw;
		this->self = this->networking->setRoute(this->self, args, &err);
		if (err.length()) {
			logError() << "route setup failed:" << err << this->m_ip << this->m_mask << this->m_gw << this->m_iface;
		}
	#endif
}


void UINetRoute::setSelf()
{
	#if PLC_BUILD
		if (m_src) {
			if (m_src->isComplete()) {
				doSetup();
			} else {
				connect(m_src, &UINetAddress::completed, this, &UINetRoute::doSetup);
			}
		}
	#endif
}


void UINetRoute::netAddr_onPropertyChanged()
{
	setSelf();
	emit srcChanged();
}


UINetRoute::UINetRoute(QObject *parent) : UINetAddress(parent)
{
	m_src = nullptr;
	#if PLC_BUILD
		INIT_LOGGER("uiro");
	#endif
}


UINetRoute::~UINetRoute()
{
	qDebug() << "~UINetRoute";
	#if PLC_BUILD
		if (networking) {
			networking->delRoute(self);
			self = nullptr;
		}
	#endif
}


void UINetRoute::setIface(Interface iface)
{
	#if PLC_BUILD
		Q_UNUSED(iface)
		logWarn() << "property iface unsupported; use src (UINetAddress) instead";
	#endif
}


void UINetRoute::setGateway(const QString &gw)
{
	m_gw = gw;
	setSelf();
	emit gatewayChanged();
}


void UINetRoute::setSrc(UINetAddress *src)
{
	m_src = src;
	setSelf();
	emit srcChanged();
}


QString UINetRoute::gateway()
{
	return m_gw;
}


UINetAddress *UINetRoute::src()
{
	return m_src;
}
