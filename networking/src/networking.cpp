#include "systemnetworking.h"
#include "idhcpclient.h"
#include "netlinkworker.h"
#include "networkingprovider.h"
#include "logcommon.h"
#include "platform.h"

#include <QVector>
#include <QThread>
#include <QNetworkAddressEntry>

#define ADDRESS_LABEL   "hmi"

#define scast(t,v) static_cast<t>(v)
#define safePtr(p) if (p) p

#define toC() toStdString().c_str()
#define toStr() toString().toC()

#define isInContainer(x, c) (c.find(x) != c.end())


namespace NetworkingCommon {

class NwAddress {
public:
	int iface;
	QNetworkAddressEntry addr;
	NwAddress(int iface, const QHostAddress &ip = QHostAddress::Null, const QHostAddress &mask = QHostAddress::Null) {
		this->iface = iface;
		this->addr.setIp(ip);
		this->addr.setNetmask(mask);
	}
	~NwAddress() = default;
};


class NwRoute {
public:
	int iface;
	QNetworkAddressEntry net;
	QHostAddress gw;
	QHostAddress src;
	NwRoute(int iface, const QNetworkAddressEntry &net, const QHostAddress &gw = QHostAddress::Null, const QHostAddress &src = QHostAddress::Null) {
		this->iface = iface;
		this->net = net;
		this->gw = gw;
		this->src = src;
	}
	~NwRoute() = default;
};


bool operator==(const NwAddress &k1, const NwAddress &k2)
{
	return k1.iface == k2.iface && k1.addr.ip() == k2.addr.ip() && k1.addr.netmask() == k2.addr.netmask();
}


uint qHash(const NwAddress &key, uint seed)
{
	return ::qHash(scast(int, key.iface), seed) ^ ::qHash(key.addr.ip(), seed) ^ ::qHash(key.addr.netmask(), seed) ^ 0x17a317a3;
}


bool operator==(const NwRoute &k1, const NwRoute &k2)
{
	return k1.iface == k2.iface && k1.gw == k2.gw;
}


uint qHash(const NwRoute &key, uint seed)
{
	return ::qHash(scast(int, key.iface), seed) ^ ::qHash(key.gw, seed) ^ 0x17a317a3;
}

} // namespace NetworkingCommon


static inline bool sameAddress(NwAddress *nwa, const QNetworkAddressEntry &newAddr, int newIface)
{
	bool cmn = newIface == -1 || nwa->iface == newIface;
	return cmn && nwa->addr.ip() == newAddr.ip() && nwa->addr.netmask() == newAddr.netmask();
}


static inline bool sameRoute(NwRoute *nwr, const QHostAddress &newGw, const QHostAddress &newSrc, int newIface)
{
	bool cmn = newIface == -1 || nwr->iface == newIface;
	return cmn && nwr->gw == newGw && nwr->src == newSrc;
}


// ##################################################################################################
// ######################################### Networking #############################################
// ##################################################################################################


class Iface {
public:
	QString name;
	bool available;
	bool link;
};


NwAddress *Networking::createAddress(int iface) const
{
	if (0 <= iface && iface < ifacesSize()) {
		NwAddress *nwa = new NwAddress(iface);
		this->addrPtrs.insert(nwa, true);
		return nwa;
	}
	return nullptr;
}


int Networking::addAddress(NwAddress *nwa, const AddressArgs &args) /*private*/
{
	int res = 0;
	int iface = (args.newIface == -1)? nwa->iface : args.newIface;
	auto p = this->addrUsers.find(*nwa);
	if (p == this->addrUsers.end()) { // is not added?
		RtNlAddr rtnl_addr(this->getSock());
		RtNlAddr::CompleteArg arg;
		arg.ifaceName = ifaceName(iface).toC();
		arg.ip = args.newAddr.ip().toStr();
		arg.mask = args.newAddr.netmask().toStr();
		arg.label = ADDRESS_LABEL;
		res = rtnl_addr.addComplete(arg);
		if (res == 0) { // ok
			nwa->iface = iface; // old or new
			nwa->addr = args.newAddr;
		}
	} // else users ++
	if (res == 0)
		this->addrUsers[*nwa]++;
	return res;
}


int Networking::rmAddress(NwAddress *nwa, bool forceDelete) /*private*/
{
	auto p = this->addrUsers.find(*nwa);
	if (p != this->addrUsers.end()) { // added?
		if (p.value()-1 <= 0 || forceDelete) { // we are the last user or force delete
			RtNlAddr rtnl_addr(this->getSock());
			RtNlAddr::CompleteArg arg;
			arg.ifaceName = ifaceName(nwa->iface).toC();
			arg.ip = nwa->addr.ip().toStr();
			arg.mask = nwa->addr.netmask().toStr();
			int res = rtnl_addr.delComplete(arg);
			if (res == 0) { // ok
				this->addrUsers.remove(p.key()); // nobody using it
			} else {
				return res; // error -> nothing to be done
			}
		} else {
			this->addrUsers[p.key()]--;
		}
	}
	// set empty
	nwa->addr.setIp(QHostAddress());
	nwa->addr.setNetmask(QHostAddress());
	return 0;
}


int Networking::setAddress(NwAddress *nwa, const AddressArgs &args, bool forceRefresh)
{
	Q_UNUSED(forceRefresh)
	if (nwa && args.newAddr.ip() != QHostAddress::Null && args.newAddr.netmask() != QHostAddress::Null) { // new should be specified
		if (isInContainer(nwa, this->addrPtrs)) {
			if ( sameAddress(nwa, args.newAddr, args.newIface) )
				return 0; // same is ignored
			int res;
			if (nwa->addr.ip() == QHostAddress::Null || nwa->addr.netmask() == QHostAddress::Null) { // old can be unspecified
				res = addAddress(nwa, args); // when just add new
			} else {
				res = rmAddress(nwa/* , forceRefresh */); // previously del old
				if (res == 0) { // ok
					res = addAddress(nwa, args); // when add new
				}
			}
			return res;
		}
	}
	return -EINVAL;
}


int Networking::unsetAddress(NwAddress *nwa, bool forceDelete)
{
	if (nwa) {
		if (isInContainer(nwa, this->addrPtrs)) {
			return rmAddress(nwa, forceDelete);
		}
	}
	return -EINVAL;
}


void Networking::delAddress(NwAddress *nwa)
{
	if (nwa) {
		if (isInContainer(nwa, this->addrPtrs)) {
			if (nwa->addr.ip() != QHostAddress::Null || nwa->addr.netmask() != QHostAddress::Null) {
				unsetAddress(nwa);
			}
			this->addrPtrs.remove(nwa);
		}
	}
	delete nwa;
}


NwRoute *Networking::createRoute(int iface, const QNetworkAddressEntry &netAddr) const
{
	if (0 <= iface && iface < ifacesSize()) {
		// IP can has zero length: this means that default route will be created,
		//  but if IP is specified, mask must be specified too
		if (netAddr.ip() != QHostAddress::Null && netAddr.netmask() == QHostAddress::Null) {
			return nullptr;
		}
		NwRoute *nwr = new NwRoute(iface, netAddr);
		this->routePtrs.insert(nwr, true);
		return nwr;
	}
	return nullptr;
}


int Networking::addRoute(NwRoute *nwr, const RouteArgs &args) /*private*/
{
	RtNlRoute rtnl_route(this->getSock());
	int iface = (args.newIface == -1)? nwr->iface : args.newIface;
	const char *ip = nullptr; // means default
	const char *mask = nullptr;
	if (nwr->net.ip() != QHostAddress::Null) {
		ip = nwr->net.ip().toStr();
		mask = nwr->net.netmask().toStr();
	}
	RtNlRoute::SimpleArg arg;
	arg.ifaceName = ifaceName(iface).toC();
	arg.destip = ip;
	arg.destmask = mask;
	arg.gw = args.newGw.toStr();
	arg.src = args.newSrc.toStr();
	int res = rtnl_route.addSimple(arg);
	if (res == 0) { // ok
		nwr->iface = iface; // old or new
		nwr->gw = args.newGw;
	}
	return res;
}


int Networking::rmRoute(NwRoute *nwr) /*private*/
{
	RtNlRoute rtnl_route(this->getSock());
	const char *ip = nullptr; // means default
	const char *mask = nullptr;
	if (nwr->net.ip() != QHostAddress::Null) {
		ip = nwr->net.ip().toStr();
		mask = nwr->net.netmask().toStr();
	}
	RtNlRoute::SimpleArg arg;
	arg.ifaceName = ifaceName(nwr->iface).toC();
	arg.destip = ip;
	arg.destmask = mask;
	arg.gw = nwr->gw.toStr();
	int res = rtnl_route.delSimple(arg);
	if (res == 0) { // ok
		// set empty
		nwr->gw = QHostAddress::Null;;
	}
	return res;
}


int Networking::setRoute(NwRoute *nwr, const RouteArgs &args)
{
	if (nwr && args.newGw != QHostAddress::Null) { // new should be specified
		if (isInContainer(nwr, this->routePtrs)) {
			if ( sameRoute(nwr, args.newGw, args.newSrc, args.newIface) )
				return 0; // same is ignored
			int res;
			if (nwr->gw == QHostAddress::Null) { // old can be unspecified
				res = addRoute(nwr, args); // when just add new
			} else {
				res = rmRoute(nwr); // previously del old
				if (res == 0) { // ok
					res = addRoute(nwr, args); // when add new
				}
			}
			return res;
		}
	}
	return -EINVAL;
}


int Networking::unsetRoute(NwRoute *nwr)
{
	if (nwr) {
		if (isInContainer(nwr, this->routePtrs)) {
			return rmRoute(nwr);
		}
	}
	return -EINVAL;
}


void Networking::delRoute(NwRoute *nwr)
{
	if (nwr) {
		if (isInContainer(nwr, this->routePtrs)) {
			if (nwr->gw != QHostAddress::Null) {
				unsetRoute(nwr);
			}
			this->routePtrs.remove(nwr);
		}
	}
	delete nwr;
}


void Networking::cleanUp()
{
	qDebug() << "Networking::cleanUp";
	for (auto &a : this->addrPtrs.keys()) {
		qDebug() << "Networking::unsetAddress" << a->addr.ip();
		unsetAddress(a/* , true */);
	}
	this->addrPtrs.clear();
	for (auto &r : this->routePtrs.keys()) {
		qDebug() << "Networking::unsetRoute" << r->gw;
		unsetRoute(r);
	}
	this->routePtrs.clear();
	qDebug() << "Networking::cleanUp end";
}


void Networking::onNlLinkEvent(const QString &ifaceName, bool up)
{
	Q_UNUSED(up)
	int iface = this->ifaceByName(ifaceName.toC());
	if (iface >= 0) {
		this->p_ifaceStat(iface, this->ifaces[iface].link, this->ifaces[iface].available);
		emit this->ifaceStateChanged(iface);
	}
}


void Networking::p_ifaceStat(int iface, bool &available, bool &link)
{
	RtNlLink rtnl_link(this->getSock(), ifaceName(iface).toC());
	if (rtnl_link.error == 0) {
		available = true;
		link = (rtnl_link.flags() & IFF_UP)? true : false;
	} else {
		available = false;
		link = false;
	}
}


int Networking::setIfaceLink(int iface, bool up)
{
	RtNlLink link(getSock(), ifaceName(iface).toC());
	if (link.error == 0)
		return (up)? link.up() : link.down();
	else
		return -EINVAL;
}


void Networking::ifaceStat(int iface, bool &available, bool &link) const
{
	if (0 <= iface && iface < ifacesSize()) {
		available = ifaces[iface].available;
		link = ifaces[iface].link;
	}
}


QString Networking::getMac(int iface)
{
	RtNlLink link(getSock(), ifaceName(iface).toC());
	NlAddr addr(link);
	char buf[32];
	return addr.toString(buf, 32);
}


QString Networking::ifaceName(int iface) const
{
	if (0 <= iface && iface < ifacesSize())
		return ifaces[iface].name;
	return nullptr;
}


int Networking::ifaceByName(const QString &iface) const
{
	int size = ifacesSize();
	for (int i = 0; i < size; ++i)
		if (ifaces[i].name == iface)
			return i;
	return -1;
}


int Networking::ifacesSize() const
{
	return ifaces.size();
}


Networking *Networking::nw_instance = nullptr;


Networking *Networking::instance(const QStringList &ifaces, QObject *parent)
{
	if (!nw_instance && ifaces.size() != 0) nw_instance = new Networking(ifaces, parent);
	return nw_instance;
}


Networking::Networking(const QStringList &ifaces, QObject *parent) : QObject(parent)
{
	sk = new NlSock();

	int size = ifaces.size();
	for (int i = 0; i < size; ++i) {
		this->ifaces.push_back({ifaces[i], false, false});
		Iface &iface = this->ifaces.last();
		p_ifaceStat(i, iface.link, iface.available);
	}

	RtNlAddr::delAllByLabel(*sk, ADDRESS_LABEL);
	RtNlRoute::delAllWithoutSrc(*sk);

	QThread *thread = new QThread();
	thread->setStackSize(0x100000);
	worker = new NetLinkWorker(this, nullptr);
	connect(worker, &NetLinkWorker::nlLinkEvent, this, &Networking::onNlLinkEvent);
	connect(thread, &QThread::started, worker, &NetLinkWorker::doWork);
	worker->moveToThread(thread);
	thread->start();
}


NlSock &Networking::getSock()
{
	return *(this->sk);
}


Networking::~Networking()
{
	qDebug() << "~Networking";
	Networking::nw_instance = nullptr;
	cleanUp();
	delete sk;

	QThread *thread;
	thread = worker->thread();
	worker->stopWork();
	thread->quit();
	thread->wait();
	delete thread;
	delete worker;
}


// ##################################################################################################
// ######################################## NetLinkWorker ###########################################
// ##################################################################################################


#define GETRTA(p,size) (struct rtattr *)(((char*)p) + NLMSG_ALIGN(size))
int NetLinkWorker::parseEvent(nl_msg_t msg, void *arg)
{
	// howto http://man7.org/linux/man-pages/man7/rtnetlink.7.html
	if (msg && arg) {
		NetLinkWorker *worker = (NetLinkWorker *)arg;
		nlmsghdr_t hdr = nlmsg_hdr(msg);
		if (hdr) {
			uint16_t type = hdr->nlmsg_type;

			if (type == RTM_NEWLINK || type == RTM_DELLINK) {
				ifinfomsg_t *ifi = (ifinfomsg_t *)NLMSG_DATA(hdr);
				rtattr_t *rth = GETRTA(ifi, sizeof(ifinfomsg_t));
				int rtl = NLMSG_PAYLOAD(hdr, sizeof(ifinfomsg_t));

				while (rtl && RTA_OK(rth, rtl)) {
					uint8_t *rta_data = (uint8_t *)RTA_DATA(rth);

					switch (rth->rta_type) {
					case IFLA_IFNAME: {
						emit worker->nlLinkEvent((const char *)rta_data,
								(ifi->ifi_flags & IFF_UP)? true : false);
						return 0;
					} break;
					default: break;
					}

					rth = RTA_NEXT(rth, rtl);
				}
			}

			// unused
			#if 0
			if (type == RTM_NEWADDR || type == RTM_DELADDR) {
				ifaddrmsg_t *ifa = (ifaddrmsg_t *)NLMSG_DATA(hdr);
				rtattr_t *rth = (rtattr_t *)IFA_RTA(ifa);
				int rtl = IFA_PAYLOAD(hdr);

				const char *iface = nullptr;
				const char *ip = nullptr;
				while (rtl && RTA_OK(rth, rtl)) {
					uint8_t *rta_data = (uint8_t *)RTA_DATA(rth);

					switch (rth->rta_type) {
					case IFA_LABEL: {
						iface = (const char *)rta_data;
					} break;
					case IFA_LOCAL: {
						ip = (const char *)rta_data;
					} break;
					default: break;
					}

					if (iface && ip) {
						char ip_cstr[INET_ADDRSTRLEN];
						char mask_cstr[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, ip, ip_cstr, INET_ADDRSTRLEN);
						prefixToMask(ifa->ifa_prefixlen, mask_cstr);
						QNetworkAddressEntry addr;
						addr.setIp(QHostAddress(ip_cstr));
						addr.setNetmask(QHostAddress(mask_cstr));
						emit worker->nlAddrEvent(iface, addr,
								(type == RTM_NEWADDR)? true : false, ifa->ifa_flags);
						return 0;
					}

					rth = RTA_NEXT(rth, rtl);
				}
			}
			#endif

			// unused
			#if 0
			if (type == RTM_NEWROUTE || type == RTM_DELROUTE) {
				rtmsg_t *rtm = (rtmsg_t *)NLMSG_DATA(hdr);
				rtattr_t *rth = GETRTA(rtm, sizeof(rtmsg_t));
				int rtl = NLMSG_PAYLOAD(hdr, sizeof(rtmsg_t));

				int iface = 0;
				const char *ip = nullptr;
				const char *gw = nullptr;
				while (rtl && RTA_OK(rth, rtl)) {
					uint8_t *rta_data = (uint8_t *)RTA_DATA(rth);

					switch (rth->rta_type) {
					case RTA_OIF: {
						iface = *((int *)rta_data);
					} break;
					case RTA_DST: {
						ip = (const char *)rta_data;
					} break;
					case RTA_GATEWAY: {
						gw = (const char *)rta_data;
					} break;
					default: break;
					}

					rth = RTA_NEXT(rth, rtl);
				}
				if (iface && gw) {
					char ifacestr[IF_NAMESIZE];
					char ip_cstr[INET_ADDRSTRLEN];
					char gw_cstr[INET_ADDRSTRLEN];
					char mask_cstr[INET_ADDRSTRLEN];
					const char *pip = nullptr;
					const char *pmask = nullptr;
					if (ip) {
						inet_ntop(AF_INET, ip, ip_cstr, INET_ADDRSTRLEN);
						prefixToMask(rtm->rtm_dst_len, mask_cstr);
						pip = ip_cstr;
						pmask = mask_cstr;
					}
					inet_ntop(AF_INET, gw, gw_cstr, INET_ADDRSTRLEN);
					QNetworkAddressEntry addr;
					addr.setIp(QHostAddress(pip));
					addr.setNetmask(QHostAddress(pmask));
					emit worker->nlRouteEvent(if_indextoname(iface, ifacestr),
							addr, gw_cstr,
							(rtm->rtm_protocol == RTPROT_DHCP)? true : false,
							(type == RTM_NEWROUTE)? true : false);
					return 0;
				}
			}
			#endif
		}
	}
	return 0;
}


void NetLinkWorker::doWork()
{
	evSk->loop();
	qDebug() << "NetLinkWorker::doWork() done";
}


void NetLinkWorker::stopWork()
{
	evSk->stopLoop();
}


NlEventSock &NetLinkWorker::getSock()
{
	return *(this->evSk);
}


NetLinkWorker::NetLinkWorker(Networking *net, QObject *parent) : QObject(parent)
{
	this->net = net;
	evSk = new NlEventSock(parseEvent, this);
	evSk->addMembership(RTNLGRP_LINK, 0);
	// these events will be handled thru sysnetworking signals
	//evSk->addMembership(RTNLGRP_IPV4_IFADDR, 0);
	//evSk->addMembership(RTNLGRP_IPV4_ROUTE, 0);
}


NetLinkWorker::~NetLinkWorker()
{
	qDebug() << "~NetLinkWorker";
	delete evSk;
}


// ##################################################################################################
// ###################################### SystemNetworking ##########################################
// ##################################################################################################


class NwObject {
public:
	IPMode ipMode;
	NwAddress *addr;
	NwRoute *route;
	NwObject() = default;
	~NwObject() = default;
};


void SystemNetworking::p_sysIfaceIpMode(int iface, IPMode mode, bool set)
{
	NwObject &nwo = this->curNwObjs[iface];
	NwAddress *nwa = curNwObjs[iface].addr;
	NwAddress *snwa = stcNwObjs[iface].addr;
	NwRoute *nwr = curNwObjs[iface].route;
	NwRoute *snwr = stcNwObjs[iface].route;

	this->networking->unsetAddress(nwa/* , true */); // force delete prev address
	this->networking->unsetRoute(nwr);
	safePtr(dhcp)->stopDhcpDaemon( networking->ifaceName(iface) );

	if (set) {
		switch (mode) {
		case IPMode::Dhcp: {
			safePtr(dhcp)->runDhcpDaemon( networking->ifaceName(iface) );
		}   // no break;
		case IPMode::Static: {
			Networking::AddressArgs aargs;
			Networking::RouteArgs rargs;
			aargs.newAddr = snwa->addr;
			rargs.newGw = snwr->gw;
			// dhcp will remove static on bound
			this->networking->setAddress(nwa, aargs/* , true */); // force set static address
			this->networking->setRoute(nwr, rargs);
		} break;
		default: break;
		}
	}

	nwo.ipMode = mode;
}


void SystemNetworking::setIfaceLink(int iface, bool up)
{
	if (0 <= iface && iface < networking->ifacesSize()) {
		if (up) {
			this->networking->setIfaceLink(iface, true);
			// ip/route will be set in @ref onIfaceStateChanged
		} else {
			NwObject &nwo = this->curNwObjs[iface];
			p_sysIfaceIpMode(iface, nwo.ipMode, false);
			this->networking->setIfaceLink(iface, false);
		}
		// signal will be provided from netlink @ref onIfaceStateChanged
	}
}


void SystemNetworking::setSysIfaceIpMode(int iface, IPMode mode)
{
	if (0 <= iface && iface < networking->ifacesSize()) {
		NwObject &nwo = this->curNwObjs[iface];
		if (nwo.ipMode != mode) {
			bool available;
			bool link;
			this->networking->ifaceStat(iface, available, link);
			p_sysIfaceIpMode(iface, mode, available);
			emit ifaceSysIpModeChanged(iface, mode);
		}
	}
}


void SystemNetworking::setAddress(int iface, const QNetworkAddressEntry &addr, QString *error)
{
	if (0 <= iface && iface < networking->ifacesSize()) {
		NwObject &nwo = curNwObjs[iface];
		NwAddress *nwa = curNwObjs[iface].addr;
		NwAddress *snwa = stcNwObjs[iface].addr;
		if (!sameAddress(snwa, addr, iface)) {
			snwa->addr = addr;
			if (nwo.ipMode == IPMode::Static) {
				Networking::AddressArgs aargs;
				aargs.newAddr = addr;
				int code = this->networking->setAddress(nwa, aargs/* , true */); // force delete previous address
				if (code != 0 && error) {
					*error = strerror(-code);
				}
				emit addressChanged(iface);
			}
		}
	}
}


void SystemNetworking::setRoute(int iface, const QHostAddress &gateway, QString *error)
{
	if (0 <= iface && iface < networking->ifacesSize()) {
		NwObject &nwo = curNwObjs[iface];
		NwRoute *nwr = curNwObjs[iface].route;
		NwRoute *snwr = stcNwObjs[iface].route;
		if (!sameRoute(snwr, gateway, QHostAddress::Null, iface)) {
			snwr->gw = gateway;
			if (nwo.ipMode == IPMode::Static) {
				Networking::RouteArgs rargs;
				rargs.newGw = gateway;
				int code = this->networking->setRoute(nwr, rargs);
				if (code != 0 && error) {
					*error = strerror(-code);
				}
				emit routeChanged(iface);
			}
		}
	}
}


void SystemNetworking::onDhcpProviderNotify(const QString &ifaceName, const QNetworkAddressEntry &addr, const QHostAddress &defGateway, bool add)
{
	int iface = networking->ifaceByName(ifaceName);
	if (0 <= iface && iface < networking->ifacesSize()) {
		NwObject &nwo = this->curNwObjs[iface];
		if (nwo.ipMode == IPMode::Dhcp) {
			if (add) {
				Networking::AddressArgs aargs;
				Networking::RouteArgs rargs;
				aargs.newAddr = addr;
				rargs.newGw = defGateway;
				this->networking->setAddress(nwo.addr, aargs/* , true */);
				this->networking->setRoute(nwo.route, rargs);
			} else {
				this->networking->unsetAddress(nwo.addr/* , true */);
				this->networking->unsetRoute(nwo.route);
			}
		}
	}
}


void SystemNetworking::onIfaceStateChanged(int iface)
{
	bool available;
	bool link;
	NwObject &nwo = this->curNwObjs[iface];
	this->networking->ifaceStat(iface, available, link);
	if (available && link) {
		p_sysIfaceIpMode(iface, nwo.ipMode, true);
	}
	emit ifaceStateChanged(iface);
}


IPMode SystemNetworking::ifaceSysIpMode(int iface) const
{
	if (0 <= iface && iface < networking->ifacesSize()) {
		return this->curNwObjs[iface].ipMode;
	}
	return IPMode::ipmnDef;
}


bool SystemNetworking::address(int iface, QNetworkAddressEntry &addr) const
{
	if (0 <= iface && iface < networking->ifacesSize()) {
		NwAddress *nwa = curNwObjs[iface].addr;
		addr = nwa->addr;
		return true;
	}
	return false;
}


bool SystemNetworking::route(int iface, QHostAddress &gateway) const
{
	if (0 <= iface && iface < networking->ifacesSize()) {
		NwRoute *nwr = curNwObjs[iface].route;
		gateway = nwr->gw;
		return true;
	}
	return false;
}


void SystemNetworking::cleanUp()
{
	networking->cleanUp();
}


QString SystemNetworking::getMac(int iface) const
{
	if (0 <= iface && iface < networking->ifacesSize()) {
		return networking->getMac(iface);
	}
	return QString();
}


bool SystemNetworking::setDhcpProvider(IDhcpClient *dhcp)
{
	if (dhcp) {
		this->dhcp = dhcp;
		connect(dhcp, &IDhcpClient::notificationReceived, this, &SystemNetworking::onDhcpProviderNotify);
		return true;
	}
	return false;
}


SystemNetworking *SystemNetworking::snw_instance = nullptr;


SystemNetworking *SystemNetworking::instance(QObject *object, const QStringList &ifaces)
{
	if (object) {
		if (!snw_instance && ifaces.size() != 0) {
			snw_instance = new SystemNetworking(ifaces, object);
		} else {
			return nullptr;
		}
		return (object == snw_instance->sysObject)? snw_instance : nullptr;
	}
	return nullptr;
}


SystemNetworking::SystemNetworking(const QStringList &ifaces, QObject *sysObject) : QObject(sysObject)
{
	this->sysObject = sysObject;
	this->networking = Networking::instance(ifaces, this);
	connect(networking, &Networking::ifaceStateChanged, this, &SystemNetworking::onIfaceStateChanged);

	this->dhcp = nullptr;

	int size = networking->ifacesSize();
	curNwObjs.resize(size); // real config
	stcNwObjs.resize(size); // static config, only storage
	for (int i = 0; i < size; ++i) {
		// empty
		curNwObjs[i].addr = this->networking->createAddress(i);
		curNwObjs[i].route = this->networking->createRoute(i, QNetworkAddressEntry()); // default ro
		curNwObjs[i].ipMode = IPMode::ipmnDef;
		stcNwObjs[i].addr = this->networking->createAddress(i);
		stcNwObjs[i].route = this->networking->createRoute(i, QNetworkAddressEntry()); // default ro
		stcNwObjs[i].ipMode = IPMode::ipmnDef;
	}
}


SystemNetworking::~SystemNetworking()
{
	qDebug() << "~SystemNetworking";
	SystemNetworking::snw_instance = nullptr;
	int size = networking->ifacesSize();
	for (int i = 0; i < size; ++i) {
		this->networking->delAddress(curNwObjs[i].addr);
		this->networking->delRoute(curNwObjs[i].route);
		this->networking->delAddress(stcNwObjs[i].addr);
		this->networking->delRoute(stcNwObjs[i].route);
	}
}


// ##################################################################################################
// ##################################### NetworkingProvider #########################################
// ##################################################################################################


bool NetworkingProvider::systemAddress(const QString &iface, QNetworkAddressEntry &addr) const
{
	if (system)
		return system->address( networking->ifaceByName(iface), addr);
	return false;
}


bool NetworkingProvider::systemRoute(const QString &iface, QHostAddress &gateway) const
{
	if (system)
		return system->route( networking->ifaceByName(iface), gateway);
	return false;
}


void NetworkingProvider::setIfaceState(const QString &iface, bool up)
{
	if (system)
		system->setIfaceLink( networking->ifaceByName(iface), up);
}


void NetworkingProvider::ifaceState(const QString &iface, bool &available, bool &link) const
{
	networking->ifaceStat( networking->ifaceByName(iface), available, link);
}


void NetworkingProvider::serviceIpMode(const QString &iface, IPMode &mode) const
{
	if (system)
		mode = system->ifaceSysIpMode( networking->ifaceByName(iface));
}


NwAddress *NetworkingProvider::setAddress(NwAddress *nwa, const AddressArgs &args, QString *error)
{
	if (!nwa) {
		nwa = networking->createAddress( networking->ifaceByName(args.iface));
	}
	Networking::AddressArgs aargs;
	aargs.newAddr = args.addr;
	int code = networking->setAddress(nwa, aargs);
	if (code != 0 && error) {
		*error = strerror(-code);
	}
	return nwa;
}


void NetworkingProvider::delAddress(NwAddress *nwa)
{
	networking->delAddress(nwa);
}


NwRoute *NetworkingProvider::setRoute(NwRoute *nwr, const RouteArgs &args, QString *error)
{
	if (!nwr) {
		nwr = networking->createRoute( networking->ifaceByName(args.iface), args.netAddr);
	}
	Networking::RouteArgs rargs;
	rargs.newGw = args.gateway;
	rargs.newSrc = args.src;
	int code = networking->setRoute(nwr, rargs);
	if (code != 0 && error) {
		*error = strerror(-code);
	}
	return nwr;
}


void NetworkingProvider::delRoute(NwRoute *nwr)
{
	networking->delRoute(nwr);
}


void NetworkingProvider::onAddressChanged(int iface)
{
	emit systemAddressChanged( networking->ifaceName(iface) );
}


void NetworkingProvider::onRouteChanged(int iface)
{
	emit systemRouteChanged( networking->ifaceName(iface) );
}


void NetworkingProvider::onIfaceStateChanged(int iface)
{
	emit ifaceStateChanged( networking->ifaceName(iface) );
}


void NetworkingProvider::onIfaceSysIpModeChanged(int iface, IPMode mode)
{
	Q_UNUSED(mode)
	emit this->serviceIpModeChanged( networking->ifaceName(iface) );
}


NetworkingProvider::NetworkingProvider(QObject *parent) : QObject(parent)
{
	system = SystemNetworking::snw_instance;
	if (system) // call constructor only if system object is registered -> else fatal error
		networking = Networking::instance();
	else
		lFatal("system object isn't registered!");

	if (system) {
		connect(system, &SystemNetworking::addressChanged, this, &NetworkingProvider::onAddressChanged);
		connect(system, &SystemNetworking::routeChanged, this, &NetworkingProvider::onRouteChanged);
		connect(system, &SystemNetworking::ifaceStateChanged, this, &NetworkingProvider::onIfaceStateChanged);
		connect(system, &SystemNetworking::ifaceSysIpModeChanged, this, &NetworkingProvider::onIfaceSysIpModeChanged);
	}
}


NetworkingProvider::~NetworkingProvider()
{
	qDebug() << "~NetworkingProvider";
}

