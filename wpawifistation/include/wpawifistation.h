#ifndef WIFICTL_H
#define WIFICTL_H

#include <QObject>
#include <QLinkedList>
#include "iwifistation.h"

class WifiCtlWorker;

class WpaWifiStation : public IWifiStation
{
    Q_OBJECT
public:

    WpaWifiStation(QObject *parent = nullptr);
    virtual ~WpaWifiStation();

public slots:
	virtual void restart(const QString &iface) override;
	virtual void stop() override;

	virtual void connectToAp(const IWifiStation::AccessPoint &ap) override;
	virtual void disconnectFromAp() override;

/* private */ signals:
    void p_setIface(const QString &iface);
    void p_started();
    void p_stopped();
    void p_connectTo(const IWifiStation::AccessPoint &ap);
    void p_disconnectFromAp();

private:
    WifiCtlWorker *worker;
};

#endif // WIFICTL_H
