#ifndef HPCPROTOCOL_H
#define HPCPROTOCOL_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QTcpSocket>
#include <openssl/ossl_typ.h>


/*!
 * @def HPCP_TIMEOUT_DEFAULT
 * @brief Таймаут на повтор отправки данных по умолчанию
*/
#define HPCP_TIMEOUT_DEFAULT	10
/*!
 * @def HPCP_DELAY_SEND
 * @brief Задержка перед отправкой кадров
*/
#define HPCP_DELAY_SEND		 10
/*!
 * @def HPCP_SENDBUF_SIZE_MAX
 * @brief Максимальный размер буфера на отправку
*/
#define HPCP_SENDBUF_SIZE_MAX   1500
/*!
 * @def HPCP_ACKFAIL_TIMEOUT
 * @brief Время ожидания кадра подтверждения до разрыва соединения
*/
#define HPCP_ACKFAIL_TIMEOUT	7000


#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter) \
	private: type m_##name; \
	public slots: void setter(type t) { this->m_##name = t; } \
	public: type getter() const { return this->m_##name; }


/*!
 * @class HPCProtocol
 * @brief Реализация протокола HMI Panel Configuration Protocol
 * NOTE схема работы: клиент/сервер наследуется от HPCProtocol;
 *	  после установки соединения вызывается метод @ref HPCProtocol::restart
 *	  с передачей сокета, на котором установлено соединение.
 *	  Весь протокол реализуется на уровне класса HPCProtocol.
 *	  Данные получаются и записываются через сигналы/слоты указанные в Q_PROPERTY.
*/
class HPCProtocol : public QObject
{
	Q_OBJECT
	/*!
	 * @property isBusy
	 * @brief Флаг занятости. Если установлен, то кадры данных будут отклонены с запросом последующего повтора
	*/
	Q_PROPERTY(bool isBusy READ isBusy WRITE setIsBusy)
	/*!
	 * @property isPrivileged
	 * @brief Флаг привилегированного режима работы (введен пароль)
	*/
	Q_PROPERTY(bool isPrivileged READ isPrivileged WRITE setIsPrivileged)
	/*!
	 * @property partner
	 * @brief Адрес удаленного отправителя/получателя
	*/
	Q_PROPERTY(FrameAddress partner READ partner NOTIFY partnerChanged)
	/*!
	 * @property panelIdent
	 * @brief Идентификация панели
	*/
	Q_PROPERTY(Panel::Ident panelIdent READ panelIdent WRITE setPanelIdent NOTIFY panelIdentChanged)
	/*!
	 * @property panelEth
	 * @brief Параметры ethernet
	*/
	Q_PROPERTY(Panel::EthParams panelEth READ panelEth WRITE setPanelEth NOTIFY panelEthChanged)
	/*!
	 * @property panelState
	 * @brief Состояние панели
	*/
	Q_PROPERTY(Panel::State panelState READ panelState WRITE setPanelState NOTIFY panelStateChanged)
	/*!
	 * @property panelWifi
	 * @brief Параметры wifi
	*/
	Q_PROPERTY(Panel::WifiParams panelWifi READ panelWifi WRITE setPanelWifi NOTIFY panelWifiChanged)
	/*!
	 * @property panelShort
	 * @brief Байтовые параметры панели
	*/
	Q_PROPERTY(Panel::ShortParams panelShort READ panelShort WRITE setPanelShort NOTIFY panelShortChanged)
	/*!
	 * @property panelStrSysLog
	 * @brief Последняя запись в системном логе
	*/
	Q_PROPERTY(Panel::StrSysLog panelStrSysLog READ panelStrSysLog WRITE setPanelStrSysLog NOTIFY panelStrSysLogAdded)
	/*!
	 * @property panelStrUserLog
	 * @brief Последняя запись в пользовательском логе
	*/
	Q_PROPERTY(Panel::StrUserLog panelStrUserLog READ panelStrUserLog WRITE setPanelStrUserLog NOTIFY panelStrUserLogAdded)
    /*!
     * @property panelUserPassword
     * @brief Пароль пользователя
    */
    Q_PROPERTY(Panel::UserPassword panelUserPassword READ panelUserPassword WRITE setPanelUserPassword NOTIFY panelUserPasswordChanged)

	Q_PROPERTY_IMPLEMENTATION(bool , isBusy , isBusy , setIsBusy)
	Q_PROPERTY_IMPLEMENTATION(bool , isPrivileged , isPrivileged , setIsPrivileged)

public:

	/*!
	 * @enum IdData
	 * @brief Идентификатор кадра
	*/
	enum IdData {
		dEnd,		//!< Завершение
		dCtl,		//!< Управление панелью
		dIdent,		//!< Идентификация панели
		dEth,		//!< Сетевые параметры
		dState,		//!< Состояние панели
		dWifi,		//!< Параметры Wifi
		dShort,		//!< Короткие параметры
		dStrSysLog,	//!< Строка системного журнала
		dStrUserLog,//!< Строка пользовательского лога
        dUserPassw,	//!< Пароль пользователя
	};

	/*!
	 * @enum IdCmd
	 * @brief Идентификатор команды
	*/
	enum IdCmd {
		cEcho,			//!< Кадр эха
		cReqAll,		//!< Запросить все параметры и состояния
		cUpd,			//!< Обновление конфигурации
		cReqSysLog,		//!< Запросить весь журнал работы
		cReqUserLog,	//!< Запросить весь лог пользователя
		cReqConfig,		//!< Запросить текущую пользовательскую конфигурацию
		cEnterPrivil,	//!< Вход в привилегированный режим
		cLeavePrivil,	//!< Выход из привилегированного режима
		cOpenRoot,		//!< Открыть доступ к root (telnet, ftp, ...)
		cSysReboot,		//!< Выполнить перезапуск системы
		cSysUpdate,		//!< Выполнить обновление системы
		cEnd
	};

	/*!
	 * @struct FrameAddress
	 * @brief Адрес отправителя кадра
	*/
	struct FrameAddress {
		QString ip;
		quint16 port;
	};

	/*!
	 * @struct Panel
	 * @brief Параметры панели
	*/
	struct Panel {

		/*!
		 * @struct Ident
		 * @brief Идентификация панели
		*/
		struct Ident {
			QString NameDisp;   //!< Имя дисплея
			QString BuildVers;  //!< Версия сборки
			QString BootVers;   //!< Версия загрузчика
			QString ProjVers;   //!< Версия проекта
			QString MetaVers;   //!< Версия мета-слоя
			QString KernelVers; //!< Версия ядра
			QByteArray toBytes() const;
			int fromBytes(const quint8 *array);
		} ident;

		/*!
		 * @struct EthParams
		 * @brief Ethernet параметры панели
		*/
		struct EthParams {
			enum NetMode {
				Static,
				Dhcp
			} Mode;
			QString IP;
			QString Mask;
			QString Gateway;
			QByteArray toBytes() const;
			int fromBytes(const quint8 *array);
		} eth;

		/*!
		 * @struct State
		 * @brief Состояние панели
		*/
		struct State {
			quint64 confStatus : 2;	//!< 0-1 Статус конфигурации
										//!< 0 – В работе
										//!< 1 – Конфигурация отсутствует
										//!< 2 – Ошибка загрузки конфигурации
			quint64 wdtBlock : 1;		//!< 2 Признак блокировки WDT
			quint64 hmidat : 1;		//!< 3 Признак доступа к контроллеру по FTP и Telnet
			quint64 stateWm : 1;		//!< 4 Текущее состояние - рабочий режим (0) или в меню (1)
			quint64 backlightDisp : 1;	//!< 5 Состояние подсветки дисплея
			quint64 ethAvailable  : 1;	//!< 6 Доступность ethernet в системе
			quint64 ethLink  : 1;		//!< 7 Наличие связи по ethernet
			quint64 wifiAvailable : 1;	//!< 8 Доступность wifi в системе
			quint64 wifiLink : 1;		//!< 9 Наличие связи по wifi
			quint64 fileReadyTrigger : 1;//!< 10 Триггер готовности последнего запрошенного файла (переключение)
		} state;

		/*!
		 * @struct WifiParams
		 * @brief Wifi параметры панели
		*/
		struct WifiParams : public EthParams {
			QString Ssid;		//!< Имя точки доступа
			QString Bssid;		//!< Адрес точки доступа
			QString Uname;		//!< Имя пользователя
			QString Password;	//!< Пароль
			QByteArray toBytes(EVP_CIPHER_CTX *ctx) const;
			int fromBytes(const quint8 *array, EVP_CIPHER_CTX *ctx);
		} wifi;

		/*!
		 * @struct ShortParams
		 * @brief Байтовые параметры панели
		*/
		struct ShortParams {
			enum NetMode {
				Ethernet,
				Wifi,
				EthWifi
			} netMode;	//!< Режим работы с сетью
			quint16 backlightTO;
			enum UiMode {
				QmlConfig,
				WebBrowser,
			} uiMode;	//!< Режим отображения контента UI
			enum LogFlags { // todo
				Note = 0x01,
				Warn = 0x02,
				Error = 0x04,
				Debug = 0x20,
			} logFlags; //!< Уровень логов
			QByteArray toBytes() const;
			int fromBytes(const quint8 *array);
		} shortp;

		struct StrSysLog {
			int type;			//!< Тип сообщения @ref ShortParams::LogFlags
			quint64 timestamp;	//!< Метка времени в мс
			QString msg;		//!< Сообщение 'Object: MSG'
			QByteArray toBytes() const;
			int fromBytes(const quint8 *array);
		} strSysLog;	//!< Последняя строка системного лога

		struct StrUserLog {
			QString fileName;	//!< Имя файла
			int line;			//!< Номер строки в файле
			quint64 timestamp;	//!< Метка времени в мс
			QString msg;		//!< Сообщение
			QByteArray toBytes() const;
			int fromBytes(const quint8 *array);
		} strUserLog;	//!< Последняя строка пользовательского лога

        struct UserPassword {
            QString self;	//!< Пароль пользователя
            QByteArray toBytes(EVP_CIPHER_CTX *ctx) const;
            int fromBytes(const quint8 *array, EVP_CIPHER_CTX *ctx);
        } userPassword;
	};

	/*!
	 * @fn restart
	 * @brief Перезапуск протокола по установившемуся соединению
	*/
	bool restart(QTcpSocket *socket);
	/*!
	 * @fn close
	 * @brief Останов протокола; закрытие соединения
	*/
	void close();

	FrameAddress partner() const;
	Panel::Ident panelIdent() const;
	Panel::EthParams panelEth() const;
	Panel::State panelState() const;
	Panel::WifiParams panelWifi() const;
	Panel::ShortParams panelShort() const;
	Panel::StrSysLog panelStrSysLog() const;
	Panel::StrUserLog panelStrUserLog() const;
    Panel::UserPassword panelUserPassword() const;

	HPCProtocol(QObject *parent = nullptr);
	virtual ~HPCProtocol();

public slots:
	virtual void setPanelIdent(const Panel::Ident &ident);
	virtual void setPanelEth(const Panel::EthParams &eth);
	virtual void setPanelState(const Panel::State &state);
	virtual void setPanelWifi(const Panel::WifiParams &wifi);
	virtual void setPanelShort(const Panel::ShortParams &shortp);
	virtual void setPanelStrSysLog(const Panel::StrSysLog &strLog);
	virtual void setPanelStrUserLog(const Panel::StrUserLog &strLog);
    virtual void setPanelUserPassword(const Panel::UserPassword &password);

signals:
	/*!
	 * @fn connected
	 * @brief Сигнал установки соединения
	*/
	void connected();
	/*!
	 * @fn disconnected
	 * @brief Сигнал разрыва соединения
	*/
	void disconnected();

	void partnerChanged();
	void panelIdentChanged();
	void panelEthChanged();
	void panelStateChanged();
	void panelWifiChanged();
	void panelShortChanged();
	void panelStrSysLogAdded();
	void panelStrUserLogAdded();
    void panelUserPasswordChanged();
	/*!
	 * @fn ctlReceived
	 * @brief Сигнал получения команды управления панелью
	*/
	void ctlReceived(IdCmd cmd, const QByteArray &prm);

protected slots:
	/*!
	 * @fn ctlSend
	 * @brief Управление панелью
	*/
	void ctlSend(IdCmd cmd, const QByteArray &prm);

private slots:
	/*!
	 * @fn read
	 * @brief Слот для получения данных с соединения
	*/
	void read();

protected:
	mutable EVP_CIPHER_CTX *ctx;

private:
	Panel m_panel;
	QTcpSocket *socket;
	FrameAddress m_partner;
	QTimer *sendTimer;
	QTimer *ackFailTimer;
	QTimer *repeatTimer;
	quint8 frameWr;		 //!< Номер кадра на передачу
	QByteArray txframe;	 //!< Кадр на передачу
	int txSent;			 //!< Размер кадра, выставленного на передачу
	/*!
	 * @fn prepareDataToSend
	 * @brief Подготовка к отправке данных удаленному получателю
	*/
	void prepareDataToSend(const QByteArray &payload);
	/*!
	 * @fn sendToPartner
	 * @brief Отправка данных с установкой таймаута на ack
	*/
	void sendToPartner();
	/*!
	 * @fn dataIsReady
	 * @brief Чтение данных от удаленного отправителя
	*/
	void dataIsReady(const FrameAddress &partner, const QByteArray &payload);
};


bool operator!=(const HPCProtocol::FrameAddress &s1, const HPCProtocol::FrameAddress &s2);

#endif // HPCPROTOCOL_H
