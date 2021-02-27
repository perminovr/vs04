#include "plugin.h"
#include "hmiworker.h"
#include "hmiqmlanalyzer.h"

#include <QQuickWindow>
#include <QDebug>
#include <QUrl>
#include <QInputEvent>

#define CMD_OPEN_ROOT 		"/usr/bin/cmdopenroot.sh"
#define CMD_UPDATE_FROM_FTP "/usr/bin/cmdupdateftp.sh"

#define scast(t,v) static_cast<t>(v)
#define safePtr(p) if (p) p


static QQuickWindow *getWindow(QQmlApplicationEngine *engine)
{
	if (engine) {
		QObject *obj = engine->rootObjects().first();
		if ( strcmp(obj->metaObject()->className(), "QQuickApplicationWindow") == 0 )
			return scast(QQuickWindow *, obj);
	}
	return nullptr;
}


static void setVisible(QQmlApplicationEngine *engine, bool visible)
{
	// if (visible) {
		QQuickWindow *window = getWindow(engine);
		if (window) {
			window->setVisible(visible);
			// window->raise();
		}
	// }
}


void HMIWorker::toggleEngineVisible()
{
	if (userCfg) {
		if (activeEngine == userCfg) {
			setVisible(userCfg, false);
			activeEngine = serviceMenu;
		} else {
			setVisible(serviceMenu, false);
			activeEngine = userCfg;
		}
		setVisible(activeEngine, true);
		database->setStateWm(activeEngine == serviceMenu); // todo when reload?
	}
}


void HMIWorker::onUserCfgClosed()
{
	QQuickWindow *window = getWindow(userCfg);
	if (window && window->visibility() == QQuickWindow::Hidden) {
		activeEngine = serviceMenu;
		setVisible(activeEngine, true);
		database->setStatus(HMIDatabase::ErrConf);
		logError() << "user configuration crashed";
	}
}


void HMIWorker::onUserCfgCreated(QObject *object, const QUrl &url)
{
	Q_UNUSED(url);
	activeEngine = serviceMenu;
	if (object) {
		QQuickWindow *window = getWindow(userCfg);
		if (window) {
			// connect(window, &QQuickWindow::visibilityChanged, this, &HMIWorker::onUserCfgClosed); todo
			activeEngine = userCfg;
			setVisible(serviceMenu, false);
			database->setStatus(HMIDatabase::InWork);
		}
	}
	setVisible(activeEngine, true);
	if (database->m.status != HMIDatabase::InWork)
		database->setStatus(HMIDatabase::ErrConf);
	this->loader->setIsBusy(false);
}


void HMIWorker::shutdownUserConfig()
{
	activeEngine = serviceMenu;
	setVisible(serviceMenu, true);
	database->setStatus(HMIDatabase::NoConf);
	// cleanup
	if (userCfg) {
		userCfg->deleteLater();
		userCfg = nullptr;
	}
}


#define userCfgIsOkErrResult(msg) \
{\
	database->setStatus(HMIDatabase::ErrConf); \
	logError() << msg; \
	return false; \
}

bool HMIWorker::userCfgIsOk()
{
	// get files list
	QString path = QLatin1String("./");
	QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	HMIQmlAnalyzer analyzer;

	while (it.hasNext()) {
		QString filePath = it.next();
		QFile file(filePath);
		if (file.open(QFile::ReadOnly)) {
			QString result;
			if ( !analyzer.fileIsOk(file, result) )
				userCfgIsOkErrResult(result);
		} else {
			return false;
		}
	}
	return true;
}


void HMIWorker::loadUserConfig()
{
	QDir::setCurrent(QStringLiteral(CONFIG_WORKING_DIR));
	QFile file(QStringLiteral(CONFIG_DEFAULT_USER_QML_FILE_NAME));
	if (file.exists() && userCfgIsOk()) {
		userCfg = new QQmlApplicationEngine(this);
		connect(userCfg, &QQmlApplicationEngine::objectCreated, this, &HMIWorker::onUserCfgCreated);

		userCfg->rootContext()->setContextProperty("uiUserCtl", uiUserCtl);
		userCfg->load( QUrl(QStringLiteral(CONFIG_DEFAULT_USER_QML_FILE_NAME)) );
	} else {
		this->loader->setIsBusy(false);
	}
}


void HMIWorker::restart()
{
	logNote() << "restart";
	shutdownUserConfig();
	loadUserConfig();
}


void HMIWorker::prepDatabase()
{
	database->read();
	database->save();

	// preinstall
	database->m.logFlags = LOGFLAGS_BASE;
	database->m.status = HMIDatabase::NoConf;
	database->m.backlight = true;
	database->m.wdtBlock = false; // todo from pin, def from pin
	database->m.hmidat = false; // todo read from file
	database->m.stateWm = true;

	bool link, available;
	link = false; available = false;
	networking->ifaceState(HMINetworkingCommon::EthIface, available, link);
	database->m.ethState.available = available;
	database->m.ethState.link = link;
	link = false; available = false;
	networking->ifaceState(HMINetworkingCommon::WifiIface, available, link);
	database->m.wifiState.available = available;
	database->m.wifiState.link = link;
}


void HMIWorker::start()
{
	logDebug() << "start";

	prepDatabase();

	// emit all
	networking->setNetMode(database->netMode);
	setNwIfacePrm(HMINetworkingCommon::EthIface);
	setNwIfacePrm(HMINetworkingCommon::WifiIface);
	loader->setPanelIdent(database->toPanelIdent());
	loader->setPanelShort(database->toPanelShort());
	loader->setPanelEth(database->toPanelEth());
	loader->setPanelWifi(database->toPanelWifi());

	// lcd backlight setup
	safePtr(lcdBl)->unset();
	safePtr(lcdBl)->turnOn();
	safePtr(lcdBl)->turnOffAfter(database->backlightTO);

	// start ssdp server
	SSDP::Params prms;
	prms.status.b = 0;
	prms.name = QStringLiteral(CONFIG_PLC_NAME);
	prms.version = database->m.version.build;
	prms.ident = networking->getMac(HMINetworkingCommon::EthIface);
	prms.type = QStringLiteral("3,0,0,0.0,0"); // todo
	prms.extdata = QString("un=%1").arg(database->name);
	ssdp->updateParams(prms);
	ssdp->restart();

	// create service menu
	if (!serviceMenu) {
		serviceMenu = new QQmlApplicationEngine(this);
		serviceMenu->rootContext()->setContextProperty("uiPanelCtl", uiPanelCtl);
		serviceMenu->load( QUrl("qrc:///qml/" CONFIG_DEFAULT_QML_FILE_NAME) );
	}

	// setup params
	uiPanelCtl->setReadOnlyOnceParams(database->toUIReadOnlyOnceParams());
	uiPanelCtl->setCommonParams(database->toUICommonParams());
	uiPanelCtl->setEthernetParams(database->toUIEthernetParams());
	uiPanelCtl->setWifiParams(database->toUIWifiParams());
	uiPanelCtl->setWifiSavedApList(database->toUIWifiSavedApList());

	restart();
}


void HMIWorker::setNwIfacePrm(HMINetworkingCommon::Iface iface)
{
	HMIDatabase::NetworkingIface prm = database->toNetworkingIface(iface);
	networking->setAddress(prm.iface, prm.addr);
	networking->setRoute(prm.iface, prm.gateway);
	switch (iface) {
	case HMINetworkingCommon::EthIface:
		networking->setSysIfaceIpMode(iface, database->ethParams.netMode);
		break;
	case HMINetworkingCommon::WifiIface:
		networking->setSysIfaceIpMode(iface, database->wifiParams.netMode);
		break;
	default:
		break;
	}
}


static QStringList _ifaceList;
const QStringList &HMINetworkingCommon::getIfacesList()
{
	return _ifaceList;
}


void HMIWorker::prepForNetworking()
{
	// read only ifaces names
	database->readIfaces();
	// change names in networking iface list
    _ifaceList.clear();
    _ifaceList.reserve(2);
	_ifaceList.push_back(database->ethParams.ifaceName);
	_ifaceList.push_back(database->wifiParams.ifaceName);
}


HMIWorker::HMIWorker(QObject *parent) : QObject(parent)
{
	logger = LogHandler::instance(this, QStringLiteral(CONFIG_SYSLOG_FILE_PATH), QStringLiteral(CONFIG_USERLOG_FILE_PATH), CONFIG_LOG_SIZE_MAX);
	logger->setFlags(LOGFLAGS_BASE | LogHandler::Debug); // todo

	INIT_LOGGER("core");
	logger->setCategoryFlags(GET_LOGGER(this), LOGFLAGS_BASE | LogHandler::Debug); // todo

	QHmiPlugin::qmlRegister();

	userCfg = nullptr;
	serviceMenu = nullptr;
	activeEngine = nullptr;

	database = new HMIDatabase(this);
	{
		// networking
		{
			connect(database, &HMIDatabase::nwNetModeChanged, this, [this](){
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
				this->loader->setPanelShort(this->database->toPanelShort());
			});
			connect(database, &HMIDatabase::ethIfaceChanged, this, [this](){
				this->uiPanelCtl->setEthernetParams(this->database->toUIEthernetParams());
				this->loader->setPanelEth(this->database->toPanelEth());
			});
			connect(database, &HMIDatabase::wifiIfaceChanged, this, [this](){
				this->uiPanelCtl->setWifiParams(this->database->toUIWifiParams());
				this->loader->setPanelWifi(this->database->toPanelWifi());
			});
		}
		// loader
		{
			connect(database, &HMIDatabase::panelIdentChanged, this, [this](){
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
			});
			connect(database, &HMIDatabase::panelEthChanged, this, [this](){
				this->setNwIfacePrm(HMINetworkingCommon::EthIface);
				this->uiPanelCtl->setEthernetParams(this->database->toUIEthernetParams());
				this->loader->setPanelEth(this->database->toPanelEth()); // after setip
			});
			connect(database, &HMIDatabase::panelWifiChanged, this, [this](){
				this->setNwIfacePrm(HMINetworkingCommon::WifiIface);
				this->uiPanelCtl->setWifiParams(this->database->toUIWifiParams());
				this->uiPanelCtl->setWifiSavedApList(this->database->toUIWifiSavedApList());
				this->loader->setPanelWifi(this->database->toPanelWifi()); // after setip
			});
			connect(database, &HMIDatabase::panelShortChanged, this, [this](){
				this->networking->setNetMode(this->database->netMode);
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
				this->setBacklightTimer();
			});
		}
		// ui
		{
			connect(database, &HMIDatabase::uiCommonChanged, this, [this](){
				this->networking->setNetMode(this->database->netMode);
				this->loader->setPanelIdent(this->database->toPanelIdent());
				this->loader->setPanelShort(this->database->toPanelShort());
				this->setBacklightTimer();
			});
			connect(database, &HMIDatabase::uiEthernetChanged, this, [this](){
				this->setNwIfacePrm(HMINetworkingCommon::EthIface);
				this->loader->setPanelEth(this->database->toPanelEth());
			});
			connect(database, &HMIDatabase::uiWifiChanged, this, [this](){
				this->setNwIfacePrm(HMINetworkingCommon::WifiIface);
				this->loader->setPanelWifi(this->database->toPanelWifi());
			});
		}
		// properties
		{
			connect(database, &HMIDatabase::statusChanged, this, [this](){
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
				this->loader->setPanelState(this->database->toPanelState());
			});
			connect(database, &HMIDatabase::logFlagsChanged, this, [this](){
				// todo handle new flags
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
				this->loader->setPanelShort(this->database->toPanelShort());
			});
			connect(database, &HMIDatabase::backlightChanged, this, [this](){
				this->loader->setPanelState(this->database->toPanelState());
			});
			connect(database, &HMIDatabase::wdtBlockChanged, this, [this](){
				// todo handle wdt state
				this->loader->setPanelState(this->database->toPanelState());
			});
			connect(database, &HMIDatabase::hmidatChanged, this, [this](){
				// todo check dat from hpcp & hmiui cmd; handle dat
				this->loader->setPanelState(this->database->toPanelState());
			});
			connect(database, &HMIDatabase::stateWmChanged, this, [this](){
				this->loader->setPanelState(this->database->toPanelState());
			});
			connect(database, &HMIDatabase::fileReadyTriggerChanged, this, [this](){
				this->loader->setPanelState(this->database->toPanelState());
			});
			// changing by networking
			connect(database, &HMIDatabase::ethAvailableChanged, this, [this](){
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
				this->loader->setPanelState(this->database->toPanelState());
				this->ssdp->restart();
			});
			connect(database, &HMIDatabase::ethLinkChanged, this, [this](){
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
				this->loader->setPanelState(this->database->toPanelState());
			});
			connect(database, &HMIDatabase::wifiAvailableChanged, this, [this](){
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
				this->loader->setPanelState(this->database->toPanelState());
				this->ssdp->restart();
			});
			connect(database, &HMIDatabase::wifiLinkChanged, this, [this](){
				this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
				this->loader->setPanelState(this->database->toPanelState());
			});
		}
	}

	prepForNetworking();
	networking = HMISystemNetworking::instance(this);
	{
		connect(networking, &HMISystemNetworking::netModeChanged, this, [this](HMINetworkingCommon::NetMode mode){
			this->database->fromNetworkingNetMode(mode);
		});
		connect(networking, &HMISystemNetworking::ifaceSysIpModeChanged, this, [this](HMINetworkingCommon::Iface iface, HMINetworkingCommon::IPMode mode){
			HMIDatabase::NetworkingIface prm = this->database->toNetworkingIface(iface);
			prm.mode = mode;
			this->database->fromNetworkingIface(prm);
		});
		connect(networking, &HMISystemNetworking::addressChanged, this, [this](HMINetworkingCommon::Iface iface){
			HMIDatabase::NetworkingIface prm = this->database->toNetworkingIface(iface);
			networking->address(iface, prm.addr);
			this->database->fromNetworkingIface(prm);
		});
		connect(networking, &HMISystemNetworking::routeChanged, this, [this](HMINetworkingCommon::Iface iface){
			HMIDatabase::NetworkingIface prm = this->database->toNetworkingIface(iface);
			networking->route(iface, prm.gateway);
			this->database->fromNetworkingIface(prm);
		});
		connect(networking, &HMISystemNetworking::ifaceStateChanged, this, [this](HMINetworkingCommon::Iface iface){
			bool link = false, available = false;
			networking->ifaceState(iface, available, link);
			switch (iface) {
			case HMINetworkingCommon::EthIface:
				this->database->setEthAvailable(available);
				this->database->setEthLink(link);
				break;
			case HMINetworkingCommon::WifiIface:
				this->database->setWifiAvailable(available);
				this->database->setWifiLink(link);
				break;
			default:
				break;
			}
		});
	}

	loader = new HMIUserLoader(this);
	{
		connect(loader, &HMIUserLoader::hpcp_onPanelIdentChanged, this, [this](const HPCProtocolServer::Panel::Ident &ident){
			this->database->fromPanelIdent(ident);
		});
		connect(loader, &HMIUserLoader::hpcp_onPanelEthChanged, this, [this](const HPCProtocolServer::Panel::EthParams &eth){
			this->database->fromPanelEth(eth);
		});
		connect(loader, &HMIUserLoader::hpcp_onPanelWifiChanged, this, [this](const HPCProtocolServer::Panel::WifiParams &wifi){
			this->database->fromPanelWifi(wifi);
		});
		connect(loader, &HMIUserLoader::hpcp_onPanelShortChanged, this, [this](const HPCProtocolServer::Panel::ShortParams &shortp){
			this->database->fromPanelShort(shortp);
		});
		connect(loader, &HMIUserLoader::hpcp_onPanelStateChanged, this, [this](const HPCProtocolServer::Panel::State &state){
			this->database->fromPanelState(state);
		});
		connect(loader, &HMIUserLoader::hpcp_onRequestedFileReady, this, [this](){
			bool fileReadyTrigger = this->database->m.fileReadyTrigger;
			this->database->setFileReadyTrigger(!fileReadyTrigger);
		});
		connect(loader, &HMIUserLoader::hpcp_onCmdUpdateReceived, this, [this](){
			this->shutdownUserConfig();
			this->loader->onReadyForUpdate();
		});
		connect(loader, &HMIUserLoader::updateReady, this, [this](bool result){
			if (result) {
				this->loader->setIsBusy(true);
				this->restart();
			} else {
				logError() << "couldn't get or unpack configuration";
				this->database->setStatus(HMIDatabase::ErrConf);
			}
		});
		connect(loader, &HMIUserLoader::hpcp_privilegedModeChanged, [this](bool status){
			if (status == false) {
				system(CMD_OPEN_ROOT " close");
			}
		});
		connect(loader, &HMIUserLoader::hpcp_onCmdRebootReceived, this, [this](){
			system("reboot");
		});
		connect(loader, &HMIUserLoader::hpcp_onCmdOpenRootReceived, [this](){
			system(CMD_OPEN_ROOT);
			logWarn() << "root rights have activated";
		});
		connect(loader, &HMIUserLoader::hpcp_onCmdSysUpdReceived, this, &HMIWorker::startSysUpdate);
	}

	uiPanelCtl = new UIPanelCtl(this);
	{
		connect(uiPanelCtl, &UIPanelCtl::onApplyCommonParams, this, [this](const QVariantMap &params){
			this->database->fromUICommonParams(params);
			this->uiPanelCtl->setCommonParams(this->database->toUICommonParams());
		});
		connect(uiPanelCtl, &UIPanelCtl::onApplyEthernetParams, this, [this](const QVariantMap &params){
			this->database->fromUIEthernetParams(params);
			this->uiPanelCtl->setEthernetParams(this->database->toUIEthernetParams());
		});
		connect(uiPanelCtl, &UIPanelCtl::onApplyWifiParams, this, [this](const QVariantMap &params){
			this->database->fromUIWifiParams(params);
			this->uiPanelCtl->setWifiParams(this->database->toUIWifiParams());
		});
		connect(uiPanelCtl, &UIPanelCtl::onApplyWifiApParams, this, [this](const QVariantMap &params){
			auto &mainAp = this->database->wifiParams.mainAp;
			QString ssid = mainAp.ssid;
			QString bssid = mainAp.bssid;
			QString uname = mainAp.uname;
			QString passw = mainAp.passw;

			this->database->fromUIWifiApParams(params);
			this->uiPanelCtl->setWifiSavedApList(this->database->toUIWifiSavedApList());

			if (ssid != mainAp.ssid || bssid != mainAp.bssid || uname != mainAp.uname || passw != mainAp.passw)
				safePtr(this->wifi)->connectToAp(HMIDatabase::convert(params));
		});
		connect(uiPanelCtl, &UIPanelCtl::onWriteLog, this, [this](int type, const QString &msg){
			LogHandler::Flags logType = HMIDatabase::convert(static_cast<UIPanelCtl::SysLogType>(type));
			switch (logType) {
			case LogHandler::Note: logNote() << msg; break;
			case LogHandler::Warn: logWarn() << msg; break;
			case LogHandler::Debug: logDebug() << msg; break;
			default: logError() << msg; break;
			}
		});
        connect(uiPanelCtl, &UIPanelCtl::onWifiSearchWindowVisibleChanged, this, [this](bool visible){
            safePtr(this->wifi)->setIsScanning(visible);
        });
		connect(uiPanelCtl, &UIPanelCtl::openUserConfigRequested, this, &HMIWorker::toggleEngineVisible);
		connect(uiPanelCtl, &UIPanelCtl::originResetRequested, this, &HMIWorker::resetOrigin);
	}

	uiUserCtl = new UIUserCtl(this);
	connect(uiUserCtl, &UIUserCtl::openMenuRequested, this, &HMIWorker::toggleEngineVisible);

	if (platform::isArm()) {
		wifi = new WpaWifiStation(this);
		{// todo
			connect(wifi, &WpaWifiStation::started, this, [this](){
				logNote() << "wifi started";
				auto &mainAp = this->database->wifiParams.mainAp;
				if (mainAp.ssid.length() != 0 || mainAp.bssid.length() != 0) {
					this->wifi->connectToAp(HMIDatabase::convert(mainAp));
				}
			});
			connect(wifi, &WpaWifiStation::stopped, this, [this](){
				logNote() << "wifi stopped";
			});
			connect(wifi, &WpaWifiStation::accessPointsFound, this, [this](const QList<WpaWifiStation::AccessPoint> &aps){
				QList<QVariantMap> uiList;
				uiList.reserve(aps.size());
				logNote() << "wifi accessPointsFound" << aps.size();
				for (const auto &x : aps) {
					uiList.push_back(HMIDatabase::convert(x));
					logNote() << "BSSID:" << x.m.BSSID << "\tconnected:" << x.m.connected << "\tSSID:" << x.m.SSID;
				}
				uiPanelCtl->setWifiFoundApList(uiList);
			});
			connect(wifi, &WpaWifiStation::connected, this, [this](){
				logNote() << "wifi connected";
			});
			connect(wifi, &WpaWifiStation::disconnected, this, [this](){
				logNote() << "wifi disconnected";
			});
			connect(wifi, &WpaWifiStation::connectError, this, [this](){
				logNote() << "wifi connectError";
			});
			networking->setWifiProvider(wifi);
		}
	} else {
		wifi = nullptr;
    }

    // todo
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, [this, t](){
        int cnt = 0;
        auto prop = t->property("cnt");
        if (prop.type() == QVariant::Int) {
            int iprop = prop.toInt();
            iprop = (iprop == 10)? 3 : 10;
            t->setProperty("cnt", iprop);
            cnt = iprop;
        } else {
            t->setProperty("cnt", 10);
            cnt = 10;
        }
        QList<IWifiStation::AccessPoint> aps;
        aps.reserve(10);
        for (int i = 0; i < cnt; ++i) {
            IWifiStation::AccessPoint ap;
            strcpy(ap.m.BSSID, "00:11:22:33:44:55");
            int frequency = i;
            ap.m.frequency = (frequency < 3500)? IWifiStation::Frequency::FREQ_2_4_GHZ :
                    (frequency < 10000)? IWifiStation::Frequency::FREQ_5_GHZ :
                    IWifiStation::Frequency::FREQ_60_GHZ;
            ap.m.signalLvl = i;
            ap.m.security = IWifiStation::Security::WPA_PSK;
            QString ssid = QString("ssid_") + QString::number(i);
            strcpy(ap.m.SSID, ssid.toLocal8Bit().data());
            if (i == 7) {
                strcpy(ap.m.SSID, "");
                ap.m.security = IWifiStation::Security::None;
                ap.m.connected = true;
            }
            aps.push_back(ap);
        }
        QList<QVariantMap> uiList;
        uiList.reserve(aps.size());
        logNote() << "wifi accessPointsFound" << aps.size();
        for (const auto &x : aps) {
            uiList.push_back(HMIDatabase::convert(x));
            logNote() << "BSSID:" << x.m.BSSID << "\tconnected:" << x.m.connected << "\tSSID:" << x.m.SSID;
        }
        uiPanelCtl->setWifiFoundApList(uiList);
    });
    t->start(7000);



	ssdp = new SSDPServer(this);
	connect(ssdp, &SSDPServer::winkReceived, this, [this](){
		// todo wink
	});

	if (platform::isArm()) {
		lcdBl = new HMILcdBacklight(this);
	} else {
		lcdBl = nullptr;
	}

	connect(logger, &LogHandler::newLogMsg, this, &HMIWorker::onNewLogMsg);
	connect(logger, &LogHandler::newQmlMsg, this->loader, &HMIUserLoader::setPanelStrUserLog);
}


HMIWorker::~HMIWorker()
{
	qDebug() << "~HMIWorker";
	// QThread *thread;
	// thread = database->thread();
	// thread->quit();
	// thread->wait();
	// delete thread;
	// delete database;
}


void HMIWorker::onNewLogMsg(int type, quint64 timestamp, const QString &msg)
{
	UIPanelCtl::SysLogType logType = HMIDatabase::convert((LogHandler::Flags)type);
	this->uiPanelCtl->writeToLogList(logType, timestamp, msg);
	this->loader->setPanelStrSysLog(type, timestamp, msg);
}


void HMIWorker::setBacklightTimer()
{
	if (lcdBl) {
		if (database->backlightTO) {
			lcdBl->turnOffAfter(database->backlightTO);
		} else {
			lcdBl->unset();
			lcdBl->turnOn();
		}
	}
}


void HMIWorker::resetOrigin()
{
	this->database->resetOrigin();
	this->loader->removeConfigWait();
	this->start();
}


void HMIWorker::onInputEvent()
{
	if (lcdBl) {
		// logDebug() << "inputEventOccured";
		if (!lcdBl->state()) {
			lcdBl->turnOn();
		}
		if (database->backlightTO) { // restart on touch
			lcdBl->reset();
		}
	}
}


#include <unistd.h>
#include "qzipreader.h"

void HMIWorker::startSysUpdate()
{
	bool ok = false;
	QFile sysupfile(QStringLiteral(CONFIG_FTPD_SYSUP_PATH));
	if (sysupfile.exists()) {
		QZipReader arch(QStringLiteral(CONFIG_FTPD_ARCHIVE_PATH));
		if (arch.status() == QZipReader::NoError) {
			ok = true;
		}
	}
	if (!ok) {
		logError() << "update start failed: couldn't get updating files from zip";
		sysupfile.remove();
		return;
	}
	QFile cmd(CMD_UPDATE_FROM_FTP);
	if (cmd.exists()) {
		auto p = cmd.permissions();
		ok = (p & QFile::Permission::ExeOwner) > 0 ? true : false;
	}
	if (!ok) {
		logError() << "update start failed: " CMD_UPDATE_FROM_FTP " not exists or hasn't exec rights";
		sysupfile.remove();
		return;
	}
	int res = execl(CMD_UPDATE_FROM_FTP, CMD_UPDATE_FROM_FTP, CONFIG_FTPD_SYSUP_PATH, (const char*)NULL);
	sysupfile.remove();
	logError() << "update start failed: execl returns result=" << res;
}

