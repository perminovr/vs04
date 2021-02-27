#include "uinetaddress.h"
#include "uitypeconverter.h"

#define scast(t,v) static_cast<t>(v)


void UINetAddress::classBegin()
{}


void UINetAddress::setSelf()
{
	#if PLC_BUILD
		if (this->networking) {
			QString err;
			HMINetworkingProvider::AddressArgs args;
			args.ip = m_ip;
			args.mask = m_mask;
			args.iface = UITypeConverter::conv(m_iface);
			self = this->networking->setAddress(self, args, &err);
			if (err.length()) {
				logError() << "ip setup failed:" << err << m_ip << m_mask << m_iface;
			}
		}
	#endif
}


void UINetAddress::componentComplete()
{
	#if PLC_BUILD
		if (!this->networking) {
			this->networking = new HMINetworkingProvider(this);
		}
	#endif
	this->m_complete = true;
	setSelf();
	emit completed(this);
}


bool UINetAddress::isComplete()
{
	return m_complete;
}


UINetAddress::UINetAddress(QObject *parent) : QObject(parent)
{
	m_iface = Interface::Ethernet;
	m_complete = false;
	self = nullptr;
	#if PLC_BUILD
		INIT_LOGGER("uiaddr");
		networking = nullptr;
	#endif
}


UINetAddress::~UINetAddress()
{
	qDebug() << "~UINetAddress";
	#if PLC_BUILD
		if (networking) {
			networking->delAddress(self);
			self = nullptr;
		}
	#endif
}


void UINetAddress::setIface(Interface iface)
{
	m_iface = iface;
	setSelf();
	emit ifaceChanged();
}


UINetAddress::Interface UINetAddress::iface() const
{
	return m_iface;
}
