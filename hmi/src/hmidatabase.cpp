#include "hmidatabase.h"
#include <unistd.h>


#define scast(t,v) static_cast<t>(v)


#define CMN_SECTION "hmi/"
# define CMN_NAME "name"
# define CMN_MODE "netmode"
# define CMN_BACKLT "backlt"
# define CMN_UIMODE "uimode"
# define CMN_PASSW "passw"
#define ETH_SECTION "eth/"
# define ETH_NAME "name"
# define ETH_MODE "mode"
# define ETH_IP "ip"
# define ETH_MASK "mask"
# define ETH_GW "gw"
#define WIFI_SECTION "wifi/"
# define WIFI_NAME "name"
# define WIFI_SSID "ssid"
# define WIFI_BSSID "bssid"
# define WIFI_UNAME "uname"
# define WIFI_PASSW "passw"
# define WIFI_MODE "mode"
# define WIFI_IP "ip"
# define WIFI_MASK "mask"
# define WIFI_GW "gw"
# define WIFI_SSID_N "ssid_"
# define WIFI_BSSID_N "bssid_"
# define WIFI_UNAME_N "uname_"
# define WIFI_PASSW_N "passw_"


#define BACKLIGHTTO_IF_ZERO "Не отключать"
#define BACKLIGHTTO_APPEND " секунд"


#define readParamInit	\
	QVariant v;			\
	QString prm;

#define readParam(k, def, strg) {		\
	v = (strg)->value(k, def);			\
	if (v.type() == QVariant::String && v.toString().size())	\
		prm = v.toString();				\
	else								\
		prm = def;						\
}

#define readParamResult(k, strg) ({		\
	bool result = false;				\
	v = (strg)->value(k, "");			\
	if (v.type() == QVariant::String && v.toString().size()) {	\
		prm = v.toString();				\
		result = true;					\
	}									\
	result;								\
})

#define readParamResultAllowEmpty(k, strg) ({		\
    bool result = false;				\
    v = (strg)->value(k, "");			\
    if (v.type() == QVariant::String) {	\
        prm = v.toString();				\
        result = true;					\
    }									\
    result;								\
})


#define saveAndEmitIfTrue(cond, signal) \
	if (cond) { save(); emit signal(); }

#define netParamsBackup(type)					\
	HMIEthernetParams type##prms = {			\
		.ifaceName = this->type##Params.ifaceName,\
		.netMode = this->type##Params.netMode,	\
		.ip = this->type##Params.ip,			\
		.mask = this->type##Params.mask,		\
		.gateway = this->type##Params.gateway	\
	};

#define netParamsEmitByChanges(type, signal)				\
	saveAndEmitIfTrue(										\
		this->type##Params.netMode != type##prms.netMode ||	\
		this->type##Params.ip != type##prms.ip ||			\
		this->type##Params.mask != type##prms.mask ||		\
		this->type##Params.gateway != type##prms.gateway,	\
		signal												\
	)


QVariantMap HMIDatabase::toUIReadOnlyOnceParams() const
{
	QVariantMap ret;
	ret["ver_build"] = this->m.version.build;
	ret["ver_uboot"] = this->m.version.uboot;
	ret["ver_project"] = this->m.version.project;
	ret["ver_meta"] = this->m.version.meta;
	ret["ver_kernel"] = this->m.version.kernel;
	return ret;
}


QVariantMap HMIDatabase::toUICommonParams() const
{
	QVariantMap ret;

	ret["name"] = this->name;
	ret["netmode"] = ({
		QString tmp = QStringLiteral("");
		switch (this->netMode) {
		case HMINetworkingCommon::Ethernet: tmp = UIPanelCtl::panelNetModeEthernet(); break;
		case HMINetworkingCommon::Wifi: tmp = UIPanelCtl::panelNetModeWifi(); break;
		case HMINetworkingCommon::EthWifi: tmp = UIPanelCtl::panelNetModeEthWifi(); break;
		}
		tmp;
	});
	ret["backlight"] = (this->backlightTO)?
			QString::number(this->backlightTO) + QStringLiteral(BACKLIGHTTO_APPEND) :
			QStringLiteral(BACKLIGHTTO_IF_ZERO);
	ret["uimode"] = ({
		QString tmp = QStringLiteral("");
		switch (this->uiMode) {
		case UIPanelCtl::UiMode::QmlConfig: tmp = UIPanelCtl::uiModeQmlConfig(); break;
		case UIPanelCtl::UiMode::WebBrowser: tmp = UIPanelCtl::uiModeWebBrowser(); break;
		}
		tmp;
	});
	ret["status"] = scast(int, this->m.status);
	ret["logflags"] = scast(int, this->m.logFlags);
	ret["hmidat"] = this->m.hmidat;
	ret["eth_av"] = this->m.ethState.available;
	ret["eth_link"] = this->m.ethState.link;
	ret["wifi_av"] = this->m.wifiState.available;
	ret["wifi_link"] = this->m.wifiState.link;

	return ret;
}


#define uiCommonBackup(pref)							\
	QString pref##name = this->name;					\
	HMINetworkingCommon::NetMode pref##mode = this->netMode;	\
	int pref##backlightTO = this->backlightTO;			\
	UIPanelCtl::UiMode pref##uiMode = this->uiMode;		\
	LogHandler::Flags pref##logflags = this->m.logFlags;

#define uiCommonEmitByChanges(pref)				\
	saveAndEmitIfTrue(							\
		this->name != pref##name ||				\
		this->backlightTO != pref##backlightTO ||	\
		this->uiMode != pref##uiMode ||			\
		this->netMode != pref##mode ||			\
		this->m.logFlags != pref##logflags,		\
		uiCommonChanged							\
	)

void HMIDatabase::fromUICommonParams(const QVariantMap &params)
{
	readParamInit;

	uiCommonBackup(bckp);

	if (readParamResult("name", &params))
		this->name = prm;

	if (readParamResult("netmode", &params))
		this->netMode = ({
			HMINetworkingCommon::NetMode mode = HMINetworkingCommon::Ethernet;
			if (prm == UIPanelCtl::panelNetModeWifi()) {
				mode = HMINetworkingCommon::Wifi;
			} else if (prm == UIPanelCtl::panelNetModeEthWifi()) {
				mode = HMINetworkingCommon::EthWifi;
			}
			mode;
		});

	if (readParamResult("backlight", &params))
		this->backlightTO = prm.toUShort();

	if (readParamResult("uimode", &params))
		this->uiMode = ({
			UIPanelCtl::UiMode mode = UIPanelCtl::UiMode::QmlConfig;
			if (prm == UIPanelCtl::uiModeWebBrowser()) {
				mode = UIPanelCtl::UiMode::WebBrowser;
			}
			mode;
		});

	if (readParamResult("logflags", &params))
		this->m.logFlags = scast(LogHandler::Flags, prm.toInt());

	uiCommonEmitByChanges(bckp);
}


static QVariantMap toUINetParams(const HMIDatabase::HMIEthernetParams *net)
{
	QVariantMap ret;

	ret["mode"] = ({
		QString tmp = QStringLiteral("");
		switch (net->netMode) {
		case HMINetworkingCommon::Static: tmp = UIPanelCtl::netModeStatic(); break;
		case HMINetworkingCommon::Dhcp: tmp = UIPanelCtl::netModeDhcp(); break;
		}
		tmp;
	});
	ret["ip"] = net->ip;
	ret["mask"] = net->mask;
	ret["gw"] = net->gateway;

	return ret;
}


static void fromUINetParams(HMIDatabase::HMIEthernetParams *net, const QVariantMap &params)
{
	readParamInit;

	if (readParamResult("mode", &params))
		net->netMode = ({
			HMINetworkingCommon::IPMode mode = HMINetworkingCommon::Static;
			if (prm == UIPanelCtl::netModeDhcp()) {
				mode = HMINetworkingCommon::Dhcp;
			}
			mode;
		});

	if (readParamResult("ip", &params))
		net->ip = prm;

	if (readParamResult("mask", &params))
		net->mask = prm;

	if (readParamResult("gw", &params))
		net->gateway = prm;
}


QVariantMap HMIDatabase::toUIEthernetParams() const
{
	return toUINetParams(&this->ethParams);
}


#define uiEthBackup(pref) \
	netParamsBackup(eth)

#define uiEthEmitByChanges(pref) \
	netParamsEmitByChanges(eth, uiEthernetChanged)

void HMIDatabase::fromUIEthernetParams(const QVariantMap &params)
{
	uiEthBackup(bckp);
	fromUINetParams(&this->ethParams, params);
	uiEthEmitByChanges(bckp);
}


QVariantMap HMIDatabase::toUIWifiParams() const
{
	QVariantMap ret = toUINetParams(&this->wifiParams);

	ret["ssid"] = this->wifiParams.mainAp.ssid;
	ret["bssid"] = this->wifiParams.mainAp.bssid;
	ret["uname"] = this->wifiParams.mainAp.uname;
	ret["passw"] = this->wifiParams.mainAp.passw;

	return ret;
}


#define uiWifiBackup(pref)	\
	netParamsBackup(wifi)

#define uiWifiEmitByChanges(pref)	\
	netParamsEmitByChanges(wifi, uiWifiChanged)

void HMIDatabase::fromUIWifiParams(const QVariantMap &params)
{
	uiWifiBackup(bckp);
	fromUINetParams(&this->wifiParams, params);
	uiWifiEmitByChanges(bckp);
}


void HMIDatabase::fromUIWifiApParams(const QVariantMap &params){
	readParamInit;

	fromUINetParams(&this->wifiParams, params);

	auto &ap = this->wifiParams.mainAp;

    if (readParamResult("ssid", &params))
		ap.ssid = prm;

    if (readParamResult("bssid", &params))
        ap.bssid = prm;

    if (readParamResultAllowEmpty("uname", &params))
        ap.uname = prm;

    if (readParamResultAllowEmpty("passw", &params))
		ap.passw = prm;

	saveApToList(ap);

    saveAndEmitIfTrue(true, uiWifiChanged);
}


QList<QVariantMap> HMIDatabase::toUIWifiSavedApList() const
{
	QList<QVariantMap> ret;

	ret.reserve(CONFIG_WIFI_APN_LIST_MAX);
	for (int i = 0; i < CONFIG_WIFI_APN_LIST_MAX; ++i) {
		QVariantMap vm;
		vm["ssid"] = this->wifiParams.apnList[i].ssid;
		vm["bssid"] = this->wifiParams.apnList[i].bssid;
		vm["uname"] = this->wifiParams.apnList[i].uname;
		vm["passw"] = this->wifiParams.apnList[i].passw;
		ret.push_back(vm);
	}

	return ret;
}


// QList<QObject *> HMIDatabase::toUIWifiSavedApList() const
// {
// 	QList<QObject *> ret;

// 	ret.reserve(CONFIG_WIFI_APN_LIST_MAX);
// 	for (int i = 0; i < CONFIG_WIFI_APN_LIST_MAX; ++i) {
// 		QObject *vm = new UIWifiAccessPoint(
// 			this->wifiParams.apnList[i].ssid,
// 			this->wifiParams.apnList[i].bssid,
// 			this->wifiParams.apnList[i].uname,
// 			this->wifiParams.apnList[i].passw
// 		);
// 		ret.push_back(vm);
// 	}

// 	return ret;
// }


HPCProtocol::Panel::Ident HMIDatabase::toPanelIdent() const
{
	HPCProtocol::Panel::Ident ret;

	ret.NameDisp = this->name;
	ret.BuildVers = this->m.version.build;
	ret.BootVers = this->m.version.uboot;
	ret.ProjVers = this->m.version.project;
	ret.MetaVers = this->m.version.meta;
	ret.KernelVers = this->m.version.kernel;

	return ret;
}


#define panelIdentBackup(pref)					\
	HPCProtocol::Panel::Ident pref##ident = {	\
		.NameDisp = this->name					\
	};

#define panelIdentEmitByChanges(pref)		\
	saveAndEmitIfTrue(						\
		this->name != pref##ident.NameDisp,	\
		panelIdentChanged				   	\
	)

void HMIDatabase::fromPanelIdent(const HPCProtocol::Panel::Ident &ident)
{
	panelIdentBackup(bckp);

	this->name = ident.NameDisp;

	if (ident.NameDisp.size() > UIPanelCtl::panelNameMaxLen())
		this->name.resize( UIPanelCtl::panelNameMaxLen() );

	panelIdentEmitByChanges(bckp);
}


static HPCProtocol::Panel::EthParams toNetParams(const HMIDatabase::HMIEthernetParams *net)
{
	HPCProtocol::Panel::EthParams ret;

	ret.Mode = scast(HPCProtocol::Panel::EthParams::NetMode, net->netMode);
	ret.IP = net->ip;
	ret.Mask = net->mask;
	ret.Gateway = net->gateway;

	return ret;
}


static void fromNetParams(HMIDatabase::HMIEthernetParams *net, const HPCProtocol::Panel::EthParams &eth)
{
	net->netMode = scast(HMINetworkingCommon::IPMode, eth.Mode);
	net->ip = eth.IP;
	net->mask = eth.Mask;
	net->gateway = eth.Gateway;
}


HPCProtocol::Panel::EthParams HMIDatabase::toPanelEth() const
{
	return toNetParams(&this->ethParams);
}


#define panelEthBackup(pref) \
	netParamsBackup(eth)

#define panelEthEmitByChanges(pref) \
	netParamsEmitByChanges(eth, panelEthChanged)

void HMIDatabase::fromPanelEth(const HPCProtocol::Panel::EthParams &eth)
{
	panelEthBackup(bckp);

	fromNetParams(&this->ethParams, eth);
	panelEthEmitByChanges(bckp);
}


HPCProtocol::Panel::State HMIDatabase::toPanelState() const
{
	HPCProtocol::Panel::State ret;

	ret.confStatus = scast(quint64, this->m.status);
	ret.wdtBlock = scast(quint64, this->m.wdtBlock);
	ret.hmidat = scast(quint64, this->m.hmidat);
	ret.stateWm = scast(quint64, this->m.stateWm);
	ret.backlightDisp = scast(quint64, this->m.backlight);
	ret.ethAvailable = scast(quint64, this->m.ethState.available);
	ret.ethLink = scast(quint64, this->m.ethState.link);
	ret.wifiAvailable = scast(quint64, this->m.wifiState.available);
	ret.wifiLink = scast(quint64, this->m.wifiState.link);
	ret.fileReadyTrigger = scast(quint64, this->m.fileReadyTrigger);

	return ret;
}


void HMIDatabase::fromPanelState(const HPCProtocol::Panel::State &state)
{
	Q_UNUSED(state); /* NOP to do */
}


HPCProtocol::Panel::WifiParams HMIDatabase::toPanelWifi() const
{
	HPCProtocol::Panel::WifiParams ret;
	HPCProtocol::Panel::EthParams eth = toNetParams(&this->wifiParams);
	ret.Mode = eth.Mode;
	ret.IP = eth.IP;
	ret.Mask = eth.Mask;
	ret.Mode = eth.Mode;

	ret.Ssid = this->wifiParams.mainAp.ssid;
	ret.Bssid = this->wifiParams.mainAp.bssid;
	ret.Uname = this->wifiParams.mainAp.uname;
	ret.Password = this->wifiParams.mainAp.passw;

	return ret;
}


#define panelWifiBackup(pref)					\
	netParamsBackup(wifi)						\
	HMIWifiParams::AccessPoint pref##apoint;	\
	pref##apoint.ssid = this->wifiParams.mainAp.ssid;	\
	pref##apoint.bssid = this->wifiParams.mainAp.bssid;	\
	pref##apoint.uname = this->wifiParams.mainAp.uname;	\
	pref##apoint.passw = this->wifiParams.mainAp.passw;

#define panelWifiEmitByChanges(pref)							\
	netParamsEmitByChanges(wifi, panelWifiChanged) else			\
	saveAndEmitIfTrue(											\
		this->wifiParams.mainAp.ssid != pref##apoint.ssid ||	\
		this->wifiParams.mainAp.bssid != pref##apoint.bssid ||	\
		this->wifiParams.mainAp.uname != pref##apoint.uname ||	\
		this->wifiParams.mainAp.passw != pref##apoint.passw,	\
		panelWifiChanged										\
	)

void HMIDatabase::fromPanelWifi(const HPCProtocol::Panel::WifiParams &wifi)
{
	panelWifiBackup(bckp);
	fromNetParams(&this->wifiParams, wifi);
	auto &ap = this->wifiParams.mainAp;
	ap.ssid = wifi.Ssid;
	ap.bssid = wifi.Bssid;
	ap.uname = wifi.Uname;
	ap.passw = wifi.Password;
	saveApToList(ap);
	panelWifiEmitByChanges(bckp);
}


HPCProtocol::Panel::ShortParams HMIDatabase::toPanelShort() const
{
	HPCProtocol::Panel::ShortParams ret;

	ret.netMode = scast(HPCProtocol::Panel::ShortParams::NetMode, this->netMode);
	ret.backlightTO = this->backlightTO;
	ret.uiMode = scast(HPCProtocol::Panel::ShortParams::UiMode, this->uiMode);
	ret.logFlags = scast(HPCProtocol::Panel::ShortParams::LogFlags, this->m.logFlags);

	return ret;
}


#define panelShortBackup(pref)														\
	HPCProtocol::Panel::ShortParams pref##shortp = {								\
		.netMode = scast(HPCProtocol::Panel::ShortParams::NetMode, this->netMode),	\
		.backlightTO = this->backlightTO,											\
		.uiMode = scast(HPCProtocol::Panel::ShortParams::UiMode, this->uiMode),		\
		.logFlags = scast(HPCProtocol::Panel::ShortParams::LogFlags, this->m.logFlags)	\
	};

#define panelShortEmitByChanges(pref)												\
	saveAndEmitIfTrue(																\
		this->netMode != scast(HMINetworkingCommon::NetMode, pref##shortp.netMode) ||	\
		this->backlightTO != pref##shortp.backlightTO ||							\
		this->uiMode != scast(UIPanelCtl::UiMode, pref##shortp.uiMode) ||			\
		this->m.logFlags != scast(LogHandler::Flags, pref##shortp.logFlags),		\
		panelShortChanged															\
	)

void HMIDatabase::fromPanelShort(const HPCProtocol::Panel::ShortParams &shortp)
{
	panelShortBackup(bckp);
	this->netMode = scast(HMINetworkingCommon::NetMode, shortp.netMode);
	this->backlightTO = shortp.backlightTO;
	this->uiMode = scast(UIPanelCtl::UiMode, shortp.uiMode);
	this->m.logFlags = scast(LogHandler::Flags, shortp.logFlags);
	panelShortEmitByChanges(bckp);
}


HPCProtocol::Panel::UserPassword HMIDatabase::toPanelUserPassword() const
{
	HPCProtocol::Panel::UserPassword ret;

	ret.self = this->userPassword;

	return ret;
}


#define panelUpassBackup(pref)							\
	HPCProtocol::Panel::UserPassword pref##upass = {	\
		.self = this->userPassword						\
	};

#define panelUpassEmitByChanges(pref)					\
	saveAndEmitIfTrue(									\
		this->userPassword != pref##upass.self,			\
		panelUserPasswordChanged						\
	)

void HMIDatabase::fromPanelUserPassword(const HPCProtocol::Panel::UserPassword &upass)
{
	panelUpassBackup(bckp);
	this->userPassword = upass.self;
	panelUpassEmitByChanges(bckp);
}


HMINetworkingCommon::NetMode HMIDatabase::toNetworkingNetMode() const
{
	return this->netMode;
}


#define nwNetModeBackup(pref) \
	HMINetworkingCommon::NetMode pref##mode = this->netMode;

#define nwNetModeEmitByChanges(pref)	\
	saveAndEmitIfTrue(					\
		this->netMode != pref##mode,	\
		nwNetModeChanged				\
	)

void HMIDatabase::fromNetworkingNetMode(HMINetworkingCommon::NetMode mode)
{
	nwNetModeBackup(bckp);
	this->netMode = mode;
	nwNetModeEmitByChanges(bckp);
}


HMIDatabase::NetworkingIface HMIDatabase::toNetworkingIface(HMINetworkingCommon::Iface iface) const
{
	NetworkingIface ret;
	ret.iface = iface;
	switch (iface) {
	case HMINetworkingCommon::EthIface:
		ret.mode = this->ethParams.netMode;
		ret.addr.setIp(QHostAddress(this->ethParams.ip));
		ret.addr.setNetmask(QHostAddress(this->ethParams.mask));
		ret.gateway = QHostAddress(this->ethParams.gateway);
		break;
	case HMINetworkingCommon::WifiIface:
		ret.mode = this->wifiParams.netMode;
		ret.addr.setIp(QHostAddress(this->wifiParams.ip));
		ret.addr.setNetmask(QHostAddress(this->wifiParams.mask));
		ret.gateway = QHostAddress(this->wifiParams.gateway);
		break;
	default:
		break;
	}
	return ret;
}


#define nwIfaceBackup(pref) \
	netParamsBackup(pref)

#define nwIfaceEmitByChanges(pref)	\
	netParamsEmitByChanges(pref, pref##IfaceChanged)

void HMIDatabase::fromNetworkingIface(const NetworkingIface &nwiface)
{
	switch (nwiface.iface) {
	case HMINetworkingCommon::EthIface: {
		nwIfaceBackup(eth);
		this->ethParams.netMode = nwiface.mode;
		this->ethParams.ip = nwiface.addr.ip().toString();
		this->ethParams.mask = nwiface.addr.netmask().toString();
		this->ethParams.gateway = nwiface.gateway.toString();
		nwIfaceEmitByChanges(eth);
	} break;
	case HMINetworkingCommon::WifiIface: {
		nwIfaceBackup(wifi);
		this->wifiParams.netMode = nwiface.mode;
		this->wifiParams.ip = nwiface.addr.ip().toString();
		this->wifiParams.mask = nwiface.addr.netmask().toString();
		this->wifiParams.gateway = nwiface.gateway.toString();
		nwIfaceEmitByChanges(wifi);
	} break;
	default:
		break;
	}
}


static QString readVersion(const QString &filename)
{
	QString ret = QStringLiteral("");
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&file);
		ret = in.readLine();
		file.close();
	} else {
		ret = QStringLiteral("0.0.0.1");
	}
	return ret;
}



bool HMIDatabase::readIfaces()
{
	readParamInit;

	readParam(ETH_SECTION ETH_NAME, "eth0", this->settings);
	this->ethParams.ifaceName = prm;

	readParam(WIFI_SECTION WIFI_NAME, "wlan0", this->settings);
	this->wifiParams.ifaceName = prm;

	return true;
}


bool HMIDatabase::read()
{
	readParamInit;

	// common

	readParam(CMN_SECTION CMN_NAME, CONFIG_PLC_NAME, this->settings);
	this->name = prm;

	readParam(CMN_SECTION CMN_MODE, "0", this->settings);
	this->netMode = scast(HMINetworkingCommon::NetMode, prm.toInt());

	readParam(CMN_SECTION CMN_BACKLT, "0", this->settings);
	this->backlightTO = prm.toUShort();

	readParam(CMN_SECTION CMN_UIMODE, "0", this->settings);
	this->uiMode = scast(UIPanelCtl::UiMode, prm.toInt());

	readParam(CMN_SECTION CMN_PASSW, "hmi", this->settings);
	this->userPassword = prm;

	// ethernet

	readParam(ETH_SECTION ETH_MODE, "0", this->settings);
	this->ethParams.netMode = scast(HMINetworkingCommon::IPMode, prm.toInt());

	readParam(ETH_SECTION ETH_IP, "10.9.32.100", this->settings);
	this->ethParams.ip = prm;

	readParam(ETH_SECTION ETH_MASK, "255.255.0.0", this->settings);
	this->ethParams.mask = prm;

	readParam(ETH_SECTION ETH_GW, "", this->settings);
	this->ethParams.gateway = prm;

	// wifi

	readParam(WIFI_SECTION WIFI_SSID, "", this->settings);
	this->wifiParams.mainAp.ssid = prm;

	readParam(WIFI_SECTION WIFI_BSSID, "", this->settings);
	this->wifiParams.mainAp.bssid = prm;

	readParam(WIFI_SECTION WIFI_UNAME, "", this->settings);
	this->wifiParams.mainAp.uname = prm;

	readParam(WIFI_SECTION WIFI_PASSW, "", this->settings);
	this->wifiParams.mainAp.passw = prm;

	readParam(WIFI_SECTION WIFI_MODE, "1", this->settings);
	this->wifiParams.netMode = scast(HMINetworkingCommon::IPMode, prm.toInt());

	readParam(WIFI_SECTION WIFI_IP, "192.168.1.50", this->settings);
	this->wifiParams.ip = prm;

	readParam(WIFI_SECTION WIFI_MASK, "255.255.255.0", this->settings);
	this->wifiParams.mask = prm;

	readParam(WIFI_SECTION WIFI_GW, "", this->settings);
	this->wifiParams.gateway = prm;

	this->wifiParams.apnList.resize(CONFIG_WIFI_APN_LIST_MAX);
	for (int i = 0; i < CONFIG_WIFI_APN_LIST_MAX; ++i) {

		readParam(WIFI_SECTION WIFI_SSID_N + QString::number(i), "", this->settings);
		this->wifiParams.apnList[i].ssid = prm;

		readParam(WIFI_SECTION WIFI_BSSID_N + QString::number(i), "", this->settings);
		this->wifiParams.apnList[i].bssid = prm;

		readParam(WIFI_SECTION WIFI_UNAME_N + QString::number(i), "", this->settings);
		this->wifiParams.apnList[i].uname = prm;

		readParam(WIFI_SECTION WIFI_PASSW_N + QString::number(i), "", this->settings);
		this->wifiParams.apnList[i].passw = prm;

	}

	// default read only params
	this->m.version.build = readVersion(CONFIG_BUILD_VER_FILE_NAME);
	this->m.version.uboot = readVersion(CONFIG_BOOT_VER_FILE_NAME);
	this->m.version.project = readVersion(CONFIG_PROJECT_VER_FILE_NAME);
	this->m.version.meta = readVersion(CONFIG_META_VER_FILE_NAME);
	this->m.version.kernel = readVersion(CONFIG_KERNEL_VER_FILE_NAME);

	return true;
}


void HMIDatabase::resetOrigin()
{
	this->settings->clear();
	this->settings->sync();
	sync();
}


void HMIDatabase::save()
{
	// common
	this->settings->setValue(CMN_SECTION CMN_NAME, this->name);
	this->settings->setValue(CMN_SECTION CMN_MODE, scast(int, this->netMode));
	this->settings->setValue(CMN_SECTION CMN_BACKLT, scast(int, this->backlightTO));
	this->settings->setValue(CMN_SECTION CMN_UIMODE, scast(int, this->uiMode));
	this->settings->setValue(CMN_SECTION CMN_PASSW, this->userPassword);
	// eth
	this->settings->setValue(ETH_SECTION ETH_NAME, this->ethParams.ifaceName);
	this->settings->setValue(ETH_SECTION ETH_MODE, scast(int, this->ethParams.netMode));
	this->settings->setValue(ETH_SECTION ETH_IP, this->ethParams.ip);
	this->settings->setValue(ETH_SECTION ETH_MASK, this->ethParams.mask);
	this->settings->setValue(ETH_SECTION ETH_GW, this->ethParams.gateway);
	// wifi
	this->settings->setValue(WIFI_SECTION WIFI_NAME, this->wifiParams.ifaceName);
	this->settings->setValue(WIFI_SECTION WIFI_SSID, this->wifiParams.mainAp.ssid);
	this->settings->setValue(WIFI_SECTION WIFI_BSSID, this->wifiParams.mainAp.bssid);
	this->settings->setValue(WIFI_SECTION WIFI_UNAME, this->wifiParams.mainAp.uname);
	this->settings->setValue(WIFI_SECTION WIFI_PASSW, this->wifiParams.mainAp.passw);
	this->settings->setValue(WIFI_SECTION WIFI_MODE, scast(int, this->wifiParams.netMode));
	this->settings->setValue(WIFI_SECTION WIFI_IP, this->wifiParams.ip);
	this->settings->setValue(WIFI_SECTION WIFI_MASK, this->wifiParams.mask);
	this->settings->setValue(WIFI_SECTION WIFI_GW, this->wifiParams.gateway);
	if (this->wifiParams.apnList.size() == CONFIG_WIFI_APN_LIST_MAX) {
		for (int i = 0; i < CONFIG_WIFI_APN_LIST_MAX; ++i) {
			this->settings->setValue(WIFI_SECTION WIFI_SSID_N + QString::number(i), this->wifiParams.apnList[i].ssid);
			this->settings->setValue(WIFI_SECTION WIFI_BSSID_N + QString::number(i), this->wifiParams.apnList[i].bssid);
			this->settings->setValue(WIFI_SECTION WIFI_UNAME_N + QString::number(i), this->wifiParams.apnList[i].uname);
			this->settings->setValue(WIFI_SECTION WIFI_PASSW_N + QString::number(i), this->wifiParams.apnList[i].passw);
		}
	}
	this->settings->sync();
	sync();
}


void HMIDatabase::saveApToList(const HMIDatabase::HMIWifiParams::AccessPoint &ap)
{
	auto p = std::find(this->wifiParams.apnList.begin(), this->wifiParams.apnList.end(), ap);
	if (p != this->wifiParams.apnList.end()) {
		*p = ap;
	} else {
		this->wifiParams.apnList.removeFirst();
		this->wifiParams.apnList.push_back(ap);
	}

}


UIPanelCtl::SysLogType HMIDatabase::convert(LogHandler::Flags type)
{
	switch (type) {
	case LogHandler::Note: return UIPanelCtl::Note;
	case LogHandler::Warn: return UIPanelCtl::Warning;
	case LogHandler::Debug: return UIPanelCtl::Debug;
	default: return UIPanelCtl::Error;
	}
}


LogHandler::Flags HMIDatabase::convert(UIPanelCtl::SysLogType type)
{
	switch (type) {
	case UIPanelCtl::Note: return LogHandler::Note;
	case UIPanelCtl::Warning: return LogHandler::Warn;
	case UIPanelCtl::Debug: return LogHandler::Debug;
	default: return LogHandler::Error;
	}
}


QVariantMap HMIDatabase::convert(const IWifiStation::AccessPoint &ap)
{
	QVariantMap ret;
	ret["ssid"] = ap.m.SSID;
	ret["bssid"] = ap.m.BSSID;
    ret["security"] = scast(int, ap.m.security);
	ret["siglvl"] = ap.m.signalLvl;
	ret["uname"] = ap.m.uname;
	ret["passw"] = ap.m.password;
	ret["fconn"] = ap.m.connected;
	return ret;
}


IWifiStation::AccessPoint HMIDatabase::convert(const QVariantMap &ap)
{
	readParamInit;

	IWifiStation::AccessPoint ret;

	if (readParamResult("ssid", &ap))
		if (prm.length()) strncpy(ret.m.SSID, prm.toLocal8Bit().data(), sizeof(ret.m.SSID)-1);

	if (readParamResult("bssid", &ap))
		if (prm.length()) strncpy(ret.m.BSSID, prm.toLocal8Bit().data(), sizeof(ret.m.BSSID)-1);

	if (readParamResult("uname", &ap))
		if (prm.length()) strncpy(ret.m.uname, prm.toLocal8Bit().data(), sizeof(ret.m.uname)-1);

	if (readParamResult("passw", &ap))
		if (prm.length()) strncpy(ret.m.password, prm.toLocal8Bit().data(), sizeof(ret.m.password)-1);

	return ret;
}



IWifiStation::AccessPoint HMIDatabase::convert(const HMIWifiParams::AccessPoint &ap)
{
	IWifiStation::AccessPoint ret;
	if (ap.ssid.length()) strncpy(ret.m.SSID, ap.ssid.toLocal8Bit().data(), sizeof(ret.m.SSID)-1);
	if (ap.bssid.length()) strncpy(ret.m.BSSID, ap.bssid.toLocal8Bit().data(), sizeof(ret.m.BSSID)-1);
	if (ap.uname.length()) strncpy(ret.m.uname, ap.uname.toLocal8Bit().data(), sizeof(ret.m.uname)-1);
	if (ap.passw.length()) strncpy(ret.m.password, ap.passw.toLocal8Bit().data(), sizeof(ret.m.password)-1);
	return ret;
}


HMIDatabase::HMIDatabase(QObject *parent) : QObject(parent)
{
	QCoreApplication::setOrganizationName("hmi");
	QCoreApplication::setOrganizationDomain("hmi.ru");
	QCoreApplication::setApplicationName("vs04");
	settings = new QSettings(QStringLiteral(CONFIG_PARAMS_FILE_PATH), QSettings::IniFormat, this);
}


HMIDatabase::~HMIDatabase()
{
	qDebug() << "~HMIDatabase";
}


int HMIDatabase::status() const
{
	return this->m.status;
}

int HMIDatabase::logFlags() const
{
	return this->m.logFlags;
}

bool HMIDatabase::backlight() const
{
	return this->m.backlight;
}

bool HMIDatabase::wdtBlock() const
{
	return this->m.wdtBlock;
}

bool HMIDatabase::hmidat() const
{
	return this->m.hmidat;
}

bool HMIDatabase::stateWm() const
{
	return this->m.stateWm;
}

bool HMIDatabase::ethAvailable() const
{
	return this->m.ethState.available;
}

bool HMIDatabase::ethLink() const
{
	return this->m.ethState.link;
}

bool HMIDatabase::wifiAvailable() const
{
	return this->m.wifiState.available;
}

bool HMIDatabase::wifiLink() const
{
	return this->m.wifiState.link;
}

bool HMIDatabase::fileReadyTrigger() const
{
	return this->m.fileReadyTrigger;
}

void HMIDatabase::setStatus(int status)
{
	HMIDatabase::Status self = scast(HMIDatabase::Status, status);
	if (this->m.status != self) {
		this->m.status = self;
		emit this->statusChanged();
	}
}

void HMIDatabase::setLogFlags(int logFlags)
{
	LogHandler::Flags self = scast(LogHandler::Flags, logFlags);
	if (this->m.logFlags != self) {
		this->m.logFlags = self;
		emit this->logFlagsChanged();
	}
}

void HMIDatabase::setBacklight(bool backlight)
{
	if (this->m.backlight != backlight) {
		this->m.backlight = backlight;
		emit this->backlightChanged();
	}
}

void HMIDatabase::setWdtBlock(bool wdtBlock)
{
	if (this->m.wdtBlock != wdtBlock) {
		this->m.wdtBlock = wdtBlock;
		emit this->wdtBlockChanged();
	}
}

void HMIDatabase::setHmidat(bool hmidat)
{
	if (this->m.hmidat != hmidat) {
		this->m.hmidat = hmidat;
		emit this->hmidatChanged();
	}
}

void HMIDatabase::setStateWm(bool stateWm)
{
	if (this->m.stateWm != stateWm) {
		this->m.stateWm = stateWm;
		emit this->stateWmChanged();
	}
}

void HMIDatabase::setEthAvailable(bool ethAvailable)
{
	if (this->m.ethState.available != ethAvailable) {
		this->m.ethState.available = ethAvailable;
		emit this->ethAvailableChanged();
	}
}

void HMIDatabase::setEthLink(bool ethLink)
{
	if (this->m.ethState.link != ethLink) {
		this->m.ethState.link = ethLink;
		emit this->ethLinkChanged();
	}
}

void HMIDatabase::setWifiAvailable(bool wifiAvailable)
{
	if (this->m.wifiState.available != wifiAvailable) {
		this->m.wifiState.available = wifiAvailable;
		emit this->wifiAvailableChanged();
	}
}

void HMIDatabase::setWifiLink(bool wifiLink)
{
	if (this->m.wifiState.link != wifiLink) {
		this->m.wifiState.link = wifiLink;
		emit this->wifiLinkChanged();
	}
}

void HMIDatabase::setFileReadyTrigger(bool fileReadyTrigger)
{
	if (this->m.fileReadyTrigger != fileReadyTrigger) {
		this->m.fileReadyTrigger = fileReadyTrigger;
		emit this->fileReadyTriggerChanged();
	}
}
