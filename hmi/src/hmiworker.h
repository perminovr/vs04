#ifndef HMIWORKER_H
#define HMIWORKER_H

#include <QObject>
#include <QTimer>

#include "config.h"
#include "platform.h"
#include "hmisystemnetworking.h"
#include "hmidatabase.h"
#include "uiuserctl.h"
#include "ssdpserver.h"
#include "loghandler.h"
#include "hmiuserloader.h"
#include "hmilcdbacklight.h"
#include "wpawifistation.h"

/*!
 * @class HMIWorker
 * @brief Основной рабочий класс
 * @details
*/
class HMIWorker : public QObject , public LoggingBase
{
	Q_OBJECT
public:
	void start();
	HMIWorker(QObject *parent = nullptr);
	virtual ~HMIWorker();

public slots:
	void restart();
	void onInputEvent();

signals:
	void finished();

private slots:
	void onUserCfgCreated(QObject *object, const QUrl &url);
	void onUserCfgClosed();
	void setBacklightTimer();
	void onNewLogMsg(int type, quint64 timestamp, const QString &msg);

	// from qml
	void toggleEngineVisible();
	void resetOrigin();
	void startSysUpdate();

private:
	QObject *application;
	LogHandler *logger;

	HMISystemNetworking *networking;
	HMIDatabase *database;

	QQmlApplicationEngine *activeEngine;
	QQmlApplicationEngine *userCfg;
	QQmlApplicationEngine *serviceMenu;

	UIPanelCtl *uiPanelCtl;
	UIUserCtl *uiUserCtl;

	IWifiStation *wifi;

	HMIUserLoader *loader;
	SSDPServer *ssdp;

	HMILcdBacklight *lcdBl;

	void shutdownUserConfig();
	void loadUserConfig();
	void loadEngine(QQmlApplicationEngine *engine, const QUrl &url);
	void setNwIfacePrm(HMINetworkingCommon::Iface iface);
	bool userCfgIsOk();
	void prepForNetworking();
	void prepDatabase();
};

#endif // HMIWORKER_H
