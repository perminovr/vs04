#ifndef HMIUSERLOADER_H
#define HMIUSERLOADER_H

#include <QObject>
#include "hpcprotocolserver.h"
#include "iftpserver.h"
#include "logcommon.h"

class HMIUserLoaderWorker;

/*!
 * @class HMIUserLoader
 * @brief Предназначен для обработки взаимодействия устройства со средой разработки:
 * @details Назначение:
 * 		- загрузка конфигурации;
 * 		- чтение / изменение параметров устройства;
*/
class HMIUserLoader : public QObject , public LoggingBase
{
	Q_OBJECT
public:

	HMIUserLoader(QObject *parent = nullptr);
	virtual ~HMIUserLoader();

	void setPanelIdent(const HPCProtocol::Panel::Ident &ident)
		{ emit this->hpcp_setPanelIdent(ident); }
	void setPanelEth(const HPCProtocol::Panel::EthParams &);
	void setPanelState(const HPCProtocol::Panel::State &);
	void setPanelWifi(const HPCProtocol::Panel::WifiParams &);
	void setPanelShort(const HPCProtocol::Panel::ShortParams &shortp)
		{ emit this->hpcp_setPanelShort(shortp); }
	void setPanelUserPassword(const HPCProtocol::Panel::UserPassword &upass)
		{ emit this->hpcp_setPanelUserPassword(upass); }
	void setPanelStrSysLog(int type, quint64 ts, const QString &strlog)
		{ emit this->hpcp_setPanelStrSysLog( {type, ts, strlog} ); }
	void setPanelStrUserLog(const QString &fileName, int line, quint64 timestamp, const QString &msg)
		{ emit this->hpcp_setPanelStrUserLog({fileName, line, timestamp, msg}); }

	void removeConfigWait();
	void setIsBusy(bool isBusy)
		{ emit this->hpcp_setIsBusy(isBusy); }
	void onReadyForUpdate()
		{ emit this->prepareUpdate(); }

signals:
	void hpcp_onPanelIdentChanged(const HPCProtocolServer::Panel::Ident &);
	void hpcp_onPanelEthChanged(const HPCProtocolServer::Panel::EthParams &);
	void hpcp_onPanelWifiChanged(const HPCProtocolServer::Panel::WifiParams &);
	void hpcp_onPanelShortChanged(const HPCProtocolServer::Panel::ShortParams &);
	void hpcp_onPanelStateChanged(const HPCProtocolServer::Panel::State &);
	void hpcp_onPanelUserPasswordChanged(const HPCProtocolServer::Panel::UserPassword &);
	void hpcp_onRequestedFileReady();
	void hpcp_onCmdUpdateReceived();
	void hpcp_privilegedModeChanged(bool status);
	void hpcp_onCmdRebootReceived();
	void hpcp_onCmdOpenRootReceived();
	void hpcp_onCmdSysUpdReceived();
	//
	void updateReady(bool result);

/* private */ signals:
	void hpcp_setPanelIdent(const HPCProtocol::Panel::Ident &ident);
	void hpcp_setPanelEth(const HPCProtocol::Panel::EthParams &eth);
	void hpcp_setPanelState(const HPCProtocol::Panel::State &state);
	void hpcp_setPanelWifi(const HPCProtocol::Panel::WifiParams &wifi);
	void hpcp_setPanelShort(const HPCProtocol::Panel::ShortParams &shortp);
	void hpcp_setPanelUserPassword(const HPCProtocol::Panel::UserPassword &upass);
	void hpcp_setPanelStrSysLog(const HPCProtocol::Panel::StrSysLog &strLog);
	void hpcp_setPanelStrUserLog(const HPCProtocol::Panel::StrUserLog &strLog);

	void hpcp_setIsBusy(bool isBusy);

	void prepareUpdate();
	void restartOnEth(const QString &ip, quint16 port);
	void restartOnWifi(const QString &ip, quint16 port);

private slots:
	void onFileReady(bool result);

private:
	HMIUserLoaderWorker *worker;
	HPCProtocolServer *ethHpcp;
	HPCProtocolServer *wifiHpcp;
	IFtpServer *ftp;
	struct IfacePrm {
		bool available;
		QString ip;
	} ethprm, wifiprm;
};

#endif // HMIUSERLOADER_H
