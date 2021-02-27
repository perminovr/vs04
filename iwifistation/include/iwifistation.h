#ifndef IWIFISTATION_H
#define IWIFISTATION_H

#include <QObject>
#include <QString>
#include <QList>

class IWifiStation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isScanning READ isScanning WRITE setIsScanning NOTIFY isScanningChanged)

public:
	enum Security {
		None,
		WPA_PSK,
		WPA_EAP
	};

	enum Frequency {
		FREQ_2_4_GHZ,
		FREQ_5_GHZ,
		FREQ_60_GHZ,
	};

	struct AccessPoint {
		struct {
			char BSSID[19];			//!< (wr) mac
			Frequency frequency;	//!< (r)
			Security security;		//!< (r)
			int signalLvl;			//!< (r)
			char SSID[33];			//!< (wr) access point name
			char uname[33];			//!< (wr) if required
			char password[33];		//!< (wr) if required
			bool connected;			//!< (r) current state
		} m;
		AccessPoint() { bzero(&m, sizeof(m)); }
		AccessPoint(const AccessPoint &ap) { memcpy(&m, &ap.m, sizeof(m)); }
		void operator=(const AccessPoint &ap) { memcpy(&m, &ap.m, sizeof(m)); }
	};

	const AccessPoint &getCurrentAp() const { return currentAp; }
    bool isScanning() const { return m_isScanning; }

    IWifiStation(QObject *parent = nullptr) : QObject(parent) {
		qRegisterMetaType<IWifiStation::AccessPoint>("IWifiStation::AccessPoint");
		qRegisterMetaType< QList<IWifiStation::AccessPoint> >("QList<IWifiStation::AccessPoint>");
	}
    virtual ~IWifiStation() {}

public slots:
    virtual void setIsScanning(bool val) {
        if (this->m_isScanning != val) {
            this->m_isScanning = val;
            emit isScanningChanged(val);
        }
    }

	virtual void restart(const QString &iface) = 0;
	virtual void start(const QString &iface) { return restart(iface); }
	virtual void stop() = 0;

	virtual void connectToAp(const AccessPoint &ap) = 0;
	virtual void disconnectFromAp() = 0;

signals:
    void isScanningChanged(bool val);
	void started();
	void stopped();
	void accessPointsFound(const QList<AccessPoint> &aps);
	void connected();
	void disconnected();
	void connectError();

protected:
	AccessPoint currentAp;
    bool m_isScanning;
};

#endif // IWIFISTATION_H
