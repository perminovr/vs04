#ifndef HMIDATABASE_H
#define HMIDATABASE_H

#include <QObject>
#include <QSettings>
#include "config.h"
#include "uipanelctl.h"
#include "hpcprotocol.h"
#include "hminetworkingcommon.h"
#include "loghandler.h"
#include "iwifistation.h"

/*!
 * @class HMIDatabase
 * @brief Основной класс для работы с параметрами устройства. Выступает связующим звеном
 * 		между службами устройства (networking, ui, cfg-loader(hpcp) )
 * @details Назначение:
 * 		- конвертация параметров из форматов служб во внутренний формат и обратно;
 * 		- инициализация параметров по старту устройства;
 * 		- запись параметров в ЭНП
*/
class HMIDatabase : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int status READ status WRITE setStatus NOTIFY statusChanged)
	Q_PROPERTY(int logFlags READ logFlags WRITE setLogFlags NOTIFY logFlagsChanged)
	Q_PROPERTY(bool backlight READ backlight WRITE setBacklight NOTIFY backlightChanged)
	Q_PROPERTY(bool wdtBlock READ wdtBlock WRITE setWdtBlock NOTIFY wdtBlockChanged)
	Q_PROPERTY(bool hmidat READ hmidat WRITE setHmidat NOTIFY hmidatChanged)
	Q_PROPERTY(bool stateWm READ stateWm WRITE setStateWm NOTIFY stateWmChanged)
	Q_PROPERTY(bool ethAvailable READ ethAvailable WRITE setEthAvailable NOTIFY ethAvailableChanged)
	Q_PROPERTY(bool ethLink READ ethLink WRITE setEthLink NOTIFY ethLinkChanged)
	Q_PROPERTY(bool wifiAvailable READ wifiAvailable WRITE setWifiAvailable NOTIFY wifiAvailableChanged)
	Q_PROPERTY(bool wifiLink READ wifiLink WRITE setWifiLink NOTIFY wifiLinkChanged)
	Q_PROPERTY(bool fileReadyTrigger READ fileReadyTrigger WRITE setFileReadyTrigger NOTIFY fileReadyTriggerChanged)

public:
	enum Status {
		InWork,
		NoConf,
		ErrConf
	};
	struct IfaceState {
		HMINetworkingCommon::Iface iface;
		bool available;	//!< Доступность интерфейса (есть в системе)
		bool link;		//!< Наличие связи (подключение на физическом/MAC уровне)
	};

	// data ==========================================================================================
	HMINetworkingCommon::NetMode netMode;   //!< Режим работы с сетью
	QString name;							//!< Имя панели
	quint16 backlightTO;					//!< Таймаут подсветки
	UIPanelCtl::UiMode uiMode;				//!< Режим отображения контента UI

	struct HMIEthernetParams {
		QString ifaceName;					//!< Системное имя интерфейса
		HMINetworkingCommon::IPMode netMode;//!< Политика установки основного IP на интерфейсе
		QString ip;							//!< Статический IP
		QString mask;						//!< Маска
		QString gateway;					//!< Шлюз по умолчанию
	} ethParams;	//!< Параметры Ethernet

	struct HMIWifiParams : public HMIEthernetParams {
		struct AccessPoint {
			QString ssid;			//!< Имя точки доступа
			QString bssid;			//!< Адрес точки доступа
			QString uname;			//!< Имя пользователя
            QString passw;			//!< Пароль
            AccessPoint() {}
            AccessPoint(const AccessPoint& ap) {
                this->ssid = ap.ssid;
                this->bssid = ap.bssid;
                this->uname = ap.uname;
                this->passw = ap.passw;
            }
			void operator=(const AccessPoint& ap) {
				this->ssid = ap.ssid;
				this->bssid = ap.bssid;
				this->uname = ap.uname;
				this->passw = ap.passw;
			}
			bool operator==(const AccessPoint& ap) {
				return this->bssid == ap.bssid;
			}
		};
		AccessPoint mainAp;				//!< Основная ТД
		QVector <AccessPoint> apnList;  //!< Список известных ТД
	} wifiParams;   //!< Параметры Wifi

	QString userPassword;			//!< Пароль пользователя

	// not stored
	struct {
		struct {
			QString build;
			QString uboot;
			QString project;
			QString meta;
			QString kernel;
		} version;				//!< Версии контроллера
		Status status;			//!< Статус работы пользовательской конфигурации
		LogHandler::Flags logFlags;	//!< Флаги уровня логов
		bool backlight;			//!< Наличие подсветки дисплея
		bool wdtBlock;			//!< Состояние блокировки WDT
		bool hmidat;			//!< Наличие сертификата
		bool stateWm;			//!< Отображаемый контент: false - пользовательская конфигурация, true - меню
		IfaceState ethState;	//!< Состояние ethernet
		IfaceState wifiState;	//!< Состояние wifi
		bool fileReadyTrigger;	//!< Переключатель готовности лога к скачиванию
	} m; //!< properties
	// end ===========================================================================================

	bool read();
	bool readIfaces();

	void save();
	void resetOrigin();

	// UI
	QVariantMap toUIReadOnlyOnceParams() const;
	QVariantMap toUICommonParams() const;
	void fromUICommonParams(const QVariantMap &);
	QVariantMap toUIEthernetParams() const;
	void fromUIEthernetParams(const QVariantMap &);
	QVariantMap toUIWifiParams() const;
	void fromUIWifiParams(const QVariantMap &);
	void fromUIWifiApParams(const QVariantMap &);
	QList<QVariantMap> toUIWifiSavedApList() const;

	// Cfg-Loader (HPCP)
	HPCProtocol::Panel::Ident toPanelIdent() const;
	void fromPanelIdent(const HPCProtocol::Panel::Ident &);
	HPCProtocol::Panel::EthParams toPanelEth() const;
	void fromPanelEth(const HPCProtocol::Panel::EthParams &);
	HPCProtocol::Panel::State toPanelState() const;
	void fromPanelState(const HPCProtocol::Panel::State &);
	HPCProtocol::Panel::WifiParams toPanelWifi() const;
	void fromPanelWifi(const HPCProtocol::Panel::WifiParams &);
	HPCProtocol::Panel::ShortParams toPanelShort() const;
	void fromPanelShort(const HPCProtocol::Panel::ShortParams &);
	HPCProtocol::Panel::UserPassword toPanelUserPassword() const;
	void fromPanelUserPassword(const HPCProtocol::Panel::UserPassword &);

	// Networking
	void fromNetworkingNetMode(HMINetworkingCommon::NetMode mode);
	HMINetworkingCommon::NetMode toNetworkingNetMode() const;
	struct NetworkingIface {
		HMINetworkingCommon::Iface iface;
		HMINetworkingCommon::IPMode mode;
		QNetworkAddressEntry addr;
		QHostAddress gateway;
	};
	void fromNetworkingIface(const NetworkingIface &nwiface);
	NetworkingIface toNetworkingIface(HMINetworkingCommon::Iface iface) const;


	int status() const;
	int logFlags() const;
	bool backlight() const;
	bool wdtBlock() const;
	bool hmidat() const;
	bool stateWm() const;
	bool ethAvailable() const;
	bool ethLink() const;
	bool wifiAvailable() const;
	bool wifiLink() const;
	bool fileReadyTrigger() const;

	HMIDatabase(QObject *parent = nullptr);
	virtual ~HMIDatabase();

	static UIPanelCtl::SysLogType convert(LogHandler::Flags type);
	static LogHandler::Flags convert(UIPanelCtl::SysLogType type);
	static QVariantMap convert(const IWifiStation::AccessPoint &ap);
	static IWifiStation::AccessPoint convert(const QVariantMap &ap);
	static IWifiStation::AccessPoint convert(const HMIWifiParams::AccessPoint &ap);

public slots:
	void setStatus(int status);
	void setLogFlags(int logFlags);
	void setBacklight(bool backlight);
	void setWdtBlock(bool wdtBlock);
	void setHmidat(bool hmidat);
	void setStateWm(bool stateWm);
	void setEthAvailable(bool ethAvailable);
	void setEthLink(bool ethLink);
	void setWifiAvailable(bool wifiAvailable);
	void setWifiLink(bool wifiLink);
	void setFileReadyTrigger(bool fileReadyTrigger);

signals:
	void uiCommonChanged();
	void uiEthernetChanged();
	void uiWifiChanged();

	void panelIdentChanged();
	void panelEthChanged();
	void panelWifiChanged();
	void panelShortChanged();
    void panelUserPasswordChanged();

	void nwNetModeChanged();
	void ethIfaceChanged();
	void wifiIfaceChanged();

	void statusChanged();
	void logFlagsChanged();
	void backlightChanged();
	void wdtBlockChanged();
	void hmidatChanged();
	void stateWmChanged();
	void ethAvailableChanged();
	void ethLinkChanged();
	void wifiAvailableChanged();
	void wifiLinkChanged();
	bool fileReadyTriggerChanged();

private:
	QSettings *settings;
	void saveApToList(const HMIWifiParams::AccessPoint &ap);
};

#endif // HMIDATABASE_H
