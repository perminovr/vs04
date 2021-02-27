#ifndef UIPANELCTL_H
#define UIPANELCTL_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QtQml>

/*!
 * @class UIPanelCtl
 * @brief Qml-синглтон, предназначенный для связывания UI системного меню с внутренней реализацией
*/
class UIPanelCtl : public QObject
{
	Q_OBJECT
	Q_ENUMS(Status)
	Q_ENUMS(SysLogType)
	Q_ENUMS(UiMode)

public:
	enum Status {
		InWork = 0,
		NoConf,
		ErrConf
	};
	enum SysLogType {
		Note = 0,
		Warning,
		Error,
		Debug
	};
	enum UiMode {
		QmlConfig,
		WebBrowser,
	};

	explicit UIPanelCtl(QObject *parent = nullptr) : QObject(parent) {}
	virtual ~UIPanelCtl() {
		qDebug() << "~UIPanelCtl";
	}

	// constants
	static Q_INVOKABLE QString netModeStatic() 			{ return QStringLiteral("Static"); }
	static Q_INVOKABLE QString netModeDhcp() 			{ return QStringLiteral("Dhcp"); }
	static Q_INVOKABLE QString panelNetModeEthernet() 	{ return QStringLiteral("Ethernet"); }
	static Q_INVOKABLE QString panelNetModeWifi() 		{ return QStringLiteral("Wifi"); }
	static Q_INVOKABLE QString panelNetModeEthWifi() 	{ return QStringLiteral("Eth+Wifi"); }
	static Q_INVOKABLE QString uiModeQmlConfig() 		{ return QStringLiteral("QmlConfig"); }
	static Q_INVOKABLE QString uiModeWebBrowser() 		{ return QStringLiteral("WebBrowser"); }
	static Q_INVOKABLE int panelNameMaxLen() 			{ return 16; }

	// from qml to core
	Q_INVOKABLE void openUserConfig() { emit openUserConfigRequested(); }
	Q_INVOKABLE void originReset() { emit originResetRequested(); }
	Q_INVOKABLE void applyCommonParams(const QVariantMap &params) { emit onApplyCommonParams(params); }
	Q_INVOKABLE void applyEthernetParams(const QVariantMap &params) { emit onApplyEthernetParams(params); }
	Q_INVOKABLE void applyWifiParams(const QVariantMap &params) { emit onApplyWifiParams(params); }
    Q_INVOKABLE void applyWifiApParams(const QVariantMap &params) { emit onApplyWifiApParams(params); }
	Q_INVOKABLE void writeLog(SysLogType type, const QString &msg) { emit onWriteLog(type, msg); }
	Q_INVOKABLE void wifiSearchWindowVisibleChanged(bool visible) { emit onWifiSearchWindowVisibleChanged(visible); }

	// from core to qml
	void setReadOnlyOnceParams(const QVariantMap &params) { emit onSetReadOnlyOnceParams(params); }
	void setCommonParams(const QVariantMap &params) { emit onSetCommonParams(params); }
	void setEthernetParams(const QVariantMap &params) { emit onSetEthernetParams(params); }
	void setWifiParams(const QVariantMap &params) { emit onSetWifiParams(params); }
//	void setWifiFoundApList(const QList<QVariantMap> &list) { emit onSetWifiFoundApList(list); } // qt 5.14 ok todo
//	void setWifiSavedApList(const QList<QVariantMap> &list) { emit onSetWifiSavedApList(list); }
	void setWifiFoundApList(const QList<QVariantMap> &list) { emit onSetWifiFoundApList( QVariant::fromValue(convert(list)) ); }
	void setWifiSavedApList(const QList<QVariantMap> &list) { emit onSetWifiSavedApList( QVariant::fromValue(convert(list)) ); }
	void writeToLogList(SysLogType type, quint64 ts, const QString &msg) { emit onWriteToLogList(type, ts, msg); }

    static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterUncreatableType<UIPanelCtl>(pkgName, mj, mi, "UIPanelCtl",
				"Can not instantiate UIPanelCtl. Please, use uiPanelCtl object instead");
	}

public slots:

signals:
	// from core to qml
	void onSetReadOnlyOnceParams(const QVariantMap &params);
	void onSetCommonParams(const QVariantMap &params);
	void onSetEthernetParams(const QVariantMap &params);
    void onSetWifiParams(const QVariantMap &params);
//	void onSetWifiFoundApList(const QList<QVariantMap> &list); // qt 5.14 ok todo
//	void onSetWifiSavedApList(const QList<QVariantMap> &list);
	void onSetWifiFoundApList(const QVariant &list);
	void onSetWifiSavedApList(const QVariant &list);
	void onWriteToLogList(int type, quint64 ts, const QString &msg);
	// from qml to core
	void openUserConfigRequested();
	void originResetRequested();
	void onApplyCommonParams(const QVariantMap &params);
	void onApplyEthernetParams(const QVariantMap &params);
	void onApplyWifiParams(const QVariantMap &params);
    void onWifiSearchWindowVisibleChanged(bool visible);
	void onApplyWifiApParams(const QVariantMap &params);
    void onWriteLog(int type, const QString &msg);

private:
	static QVariantList convert(const QList<QVariantMap> &list) {
		QVariantList ret;
		ret.reserve(list.size());
		for (const auto &x : list) {
			ret.push_back(x);
		}
		return ret;
	}
};

#endif // UIPANELCTL_H
