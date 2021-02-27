#include "hmiuserloaderworker.h"
#include "qzipreader.h"
#include "config.h"
#include "platform.h"
#include "inetdftpprovider.h"

#include <QThread>
#include <QDir>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>


bool HMIUserLoaderWorker::removeConfig()
{
	QDir dir(QStringLiteral(CONFIG_WORKING_DIR));
	bool result = dir.removeRecursively();
	if (result)
		result = dir.mkpath(".");
	return result;
}


void HMIUserLoaderWorker::prepareUpdate()
{
	bool result = false;
	{ // checking size of the config
		QFile f (QStringLiteral(CONFIG_FTPD_ARCHIVE_PATH));
		if (f.size() > CONFIG_FTPD_ARCHIVE_SIZE_MAX)
			goto exit;
	}
	{ // prep for the config extracting
		QZipReader arch(QStringLiteral(CONFIG_FTPD_ARCHIVE_PATH));
		if (arch.status() == QZipReader::NoError) {
			// clean up
			if ( !removeConfig() )
				goto exit;
			// extract
			result = arch.extractAll(QStringLiteral(CONFIG_WORKING_DIR));
			// create backup
			if (result)
				QFile::copy(QStringLiteral(CONFIG_FTPD_ARCHIVE_PATH), QStringLiteral(CONFIG_BACKUPED_ARCHIVE_PATH));
			sync();
		}
	}
exit:
	emit this->updateReady(result);
}


void HMIUserLoaderWorker::prepareSysLog()
{
	QFile oldLog(QStringLiteral(CONFIG_FTPD_WORKING_DIR "/" CONFIG_SYSLOG_FILE_NAME));
		oldLog.remove();
	bool result = QFile::copy(QStringLiteral(CONFIG_SYSLOG_FILE_PATH), QStringLiteral(CONFIG_FTPD_WORKING_DIR "/" CONFIG_SYSLOG_FILE_NAME));
	emit this->sysLogReady(result);
}


void HMIUserLoaderWorker::prepareUserLog()
{
	QFile oldLog(QStringLiteral(CONFIG_FTPD_WORKING_DIR "/" CONFIG_USERLOG_FILE_NAME));
		oldLog.remove();
	bool result = QFile::copy(QStringLiteral(CONFIG_USERLOG_FILE_PATH), QStringLiteral(CONFIG_FTPD_WORKING_DIR "/" CONFIG_USERLOG_FILE_NAME));
	emit this->userLogReady(result);
}


void HMIUserLoaderWorker::prepareConfig()
{
	QFile oldCfg(QStringLiteral(CONFIG_FTPD_ARCHIVE_PATH));
		oldCfg.remove();
	bool result = QFile::copy(QStringLiteral(CONFIG_BACKUPED_ARCHIVE_PATH), QStringLiteral(CONFIG_FTPD_ARCHIVE_PATH));
	emit this->configReady(result);
}


void HMIUserLoader::removeConfigWait()
{
	this->worker->removeConfig();
}


void HMIUserLoader::onFileReady(bool result)
{
	if (result) {
		emit this->hpcp_onRequestedFileReady();
	} else {
		logError() << "requested file does not exist";
	}
}


HMIUserLoader::HMIUserLoader(QObject *parent) : QObject(parent)
{
	QThread *thread;

	INIT_LOGGER("core");

	thread = new QThread();
	thread->setStackSize(CONFIG_THREAD_STACK_SIZE);

	worker = new HMIUserLoaderWorker();
	{
		connect(this, &HMIUserLoader::prepareUpdate, worker, &HMIUserLoaderWorker::prepareUpdate);
		connect(worker, &HMIUserLoaderWorker::sysLogReady, this, &HMIUserLoader::onFileReady);
		connect(worker, &HMIUserLoaderWorker::userLogReady, this, &HMIUserLoader::onFileReady);
		connect(worker, &HMIUserLoaderWorker::configReady, this, &HMIUserLoader::onFileReady);
		connect(worker, &HMIUserLoaderWorker::updateReady, this, &HMIUserLoader::updateReady);
	}
	worker->moveToThread(thread);

	ethHpcp = new HPCProtocolServer(this);
	wifiHpcp = new HPCProtocolServer(this);
	{
		auto initHpcp = [this](HPCProtocolServer *hpcp){
			connect(this, &HMIUserLoader::hpcp_setPanelIdent, hpcp, &HPCProtocolServer::setPanelIdent);
			connect(this, &HMIUserLoader::hpcp_setPanelEth, hpcp, &HPCProtocolServer::setPanelEth);
			connect(this, &HMIUserLoader::hpcp_setPanelState, hpcp, &HPCProtocolServer::setPanelState);
			connect(this, &HMIUserLoader::hpcp_setPanelWifi, hpcp, &HPCProtocolServer::setPanelWifi);
			connect(this, &HMIUserLoader::hpcp_setPanelShort, hpcp, &HPCProtocolServer::setPanelShort);
			connect(this, &HMIUserLoader::hpcp_setPanelUserPassword, hpcp, &HPCProtocolServer::setPanelUserPassword);
			connect(this, &HMIUserLoader::hpcp_setPanelStrSysLog, hpcp, &HPCProtocolServer::setPanelStrSysLog);
			connect(this, &HMIUserLoader::hpcp_setPanelStrUserLog, hpcp, &HPCProtocolServer::setPanelStrUserLog);
			connect(this, &HMIUserLoader::hpcp_setIsBusy, hpcp, &HPCProtocolServer::setIsBusy);
			//
			connect(hpcp, &HPCProtocolServer::connected, [this, hpcp](){
				logNote() << "user on [" << hpcp->partner().ip << "] connected";
			});
			connect(hpcp, &HPCProtocolServer::disconnected, [this, hpcp](){
				logNote() << "user on [" << hpcp->partner().ip << "] disconnected";
			});
			connect(hpcp, &HPCProtocolServer::privilegedModeChanged, [this, hpcp](bool status){
				if (status) logWarn() << "user on [" << hpcp->partner().ip << "] has activated privileged access";
				emit hpcp_privilegedModeChanged(status);
			});
			connect(hpcp, &HPCProtocolServer::privilegeError, [this, hpcp](HPCProtocol::IdCmd cmd){
				const char *scmd = "";
				switch(cmd) {
					case HPCProtocol::IdCmd::cOpenRoot: scmd = "root rights"; break;
					case HPCProtocol::IdCmd::cSysReboot: scmd = "system rebooting"; break;
					case HPCProtocol::IdCmd::cSysUpdate: scmd = "system updating"; break;
					default: break;
				}
				logError() << "user on [" << hpcp->partner().ip << "] has not privilege for" << scmd;
			});
			connect(hpcp, &HPCProtocolServer::panelIdentChanged, [this, hpcp](){
				emit this->hpcp_onPanelIdentChanged(hpcp->panelIdent());
			});
			connect(hpcp, &HPCProtocolServer::panelEthChanged, [this, hpcp](){
				emit this->hpcp_onPanelEthChanged(hpcp->panelEth());
			});
			connect(hpcp, &HPCProtocolServer::panelWifiChanged, [this, hpcp](){
				emit this->hpcp_onPanelWifiChanged(hpcp->panelWifi());
			});
			connect(hpcp, &HPCProtocolServer::panelShortChanged, [this, hpcp](){
				emit this->hpcp_onPanelShortChanged(hpcp->panelShort());
			});
			connect(hpcp, &HPCProtocolServer::panelStateChanged, [this, hpcp](){
				emit this->hpcp_onPanelStateChanged(hpcp->panelState());
			});
			connect(hpcp, &HPCProtocolServer::panelUserPasswordChanged, [this, hpcp](){
				emit this->hpcp_onPanelUserPasswordChanged(hpcp->panelUserPassword());
			});
			connect(hpcp, &HPCProtocolServer::cmdUpdateReceived, this, &HMIUserLoader::hpcp_onCmdUpdateReceived);
			connect(hpcp, &HPCProtocolServer::cmdRebootReceived, this, &HMIUserLoader::hpcp_onCmdRebootReceived);
			connect(hpcp, &HPCProtocolServer::cmdOpenRootReceived, this, &HMIUserLoader::hpcp_onCmdOpenRootReceived);
			connect(hpcp, &HPCProtocolServer::cmdSysUpdReceived, this, &HMIUserLoader::hpcp_onCmdSysUpdReceived);
			connect(hpcp, &HPCProtocolServer::fullSysLogRequested, worker, &HMIUserLoaderWorker::prepareSysLog);
			connect(hpcp, &HPCProtocolServer::fullUserLogRequested, worker, &HMIUserLoaderWorker::prepareUserLog);
			connect(hpcp, &HPCProtocolServer::configRequested, worker, &HMIUserLoaderWorker::prepareConfig);
		};
		initHpcp(ethHpcp);
		initHpcp(wifiHpcp);
		connect(this, &HMIUserLoader::restartOnEth, ethHpcp, &HPCProtocolServer::restart);
		connect(this, &HMIUserLoader::restartOnWifi, wifiHpcp, &HPCProtocolServer::restart);
	}
	// ethHpcp->moveToThread(thread);
	// wifiHpcp->moveToThread(thread);

	thread->start();

	// create ftpd working dir
	bool ok = true;
	QDir dir(CONFIG_FTPD_WORKING_DIR);
	if (!dir.exists())
		ok = dir.mkpath(".");
	if (ok) {
		ok = false;
		struct passwd *psw = getpwnam(CONFIG_FTPD_LOGIN);
		if (psw) {
			ok = (chown(CONFIG_FTPD_WORKING_DIR, psw->pw_uid, psw->pw_gid) == 0);
		}
	}
	if (!ok) {
		logError() << "Failed to create ftpd working dir";
	}

	if (platform::isArm()) {
		ftp = new InetdFtpProvider(QStringLiteral(CONFIG_FTPD_INETD_CONF), this);
		ok = ftp->runDaemon(
				QStringLiteral(CONFIG_FTPD_WORKING_DIR),
				QStringLiteral(CONFIG_FTPD_LOGIN),
				QStringLiteral(CONFIG_FTPD_PASSWORD),
				CONFIG_FTPD_PORT);
		if (!ok) {
			logError() << "Failed to start ftpd";
		}
	} else {
		ftp = nullptr;
	}
}


HMIUserLoader::~HMIUserLoader()
{
	qDebug() << "~HMIUserLoader";
	QThread *thread;
	thread = worker->thread();
	thread->quit();
	thread->wait();
	delete thread;
	delete worker;
	// delete ethHpcp;
	// delete wifiHpcp;
}


void HMIUserLoader::setPanelEth(const HPCProtocol::Panel::EthParams &eth)
{
	emit this->hpcp_setPanelEth(eth);
	if (ethprm.ip != eth.IP) {
		ethprm.ip = eth.IP;
		logDebug() << "HMIUserLoader::restartOnEth";
		emit this->restartOnEth(ethprm.ip, CONFIG_HPCP_PORT);
	}
}


void HMIUserLoader::setPanelState(const HPCProtocol::Panel::State &state)
{
	emit this->hpcp_setPanelState(state);
	if (!ethprm.available && state.ethAvailable) {
		logDebug() << "HMIUserLoader::restartOnEth";
		emit this->restartOnEth(ethprm.ip, CONFIG_HPCP_PORT);
	}
	if (!wifiprm.available && state.wifiAvailable) {
		logDebug() << "HMIUserLoader::restartOnWifi";
		emit this->restartOnWifi(wifiprm.ip, CONFIG_HPCP_PORT);
	}
	ethprm.available = state.ethAvailable;
	wifiprm.available = state.wifiAvailable;
}


void HMIUserLoader::setPanelWifi(const HPCProtocol::Panel::WifiParams &wifi)
{
	emit this->hpcp_setPanelWifi(wifi);
	if (wifiprm.ip != wifi.IP) {
		wifiprm.ip = wifi.IP;
		logDebug() << "HMIUserLoader::restartOnWifi";
		emit this->restartOnWifi(wifiprm.ip, CONFIG_HPCP_PORT);
	}
}

