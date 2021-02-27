#include <QtCore>
#include <QDebug>
#include "wpawifistation.h"

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);

    WpaWifiStation *ctl = new WpaWifiStation(&app);

    QTimer::singleShot(0, ctl, [ctl](){
		ctl->start("wlan0");
        ctl->setIsScanning(true);
	});

    QTimer::singleShot(10000, ctl, [ctl](){
		WpaWifiStation::AccessPoint ap;
		strcpy (ap.m.SSID, "hmi_test3");
		strcpy (ap.m.password, "zpsa_hmi_install");
		ctl->connectToAp(ap);
	});

    // QTimer::singleShot(20000, ctl, [ctl](){
	// 	WpaWifiStation::AccessPoint ap;
	// 	strcpy (ap.m.SSID, "eleteam1");
	// 	strcpy (ap.m.password, "eletimpwd");
	// 	ctl->connectToAp(ap);
	// });

	QObject::connect(ctl, &WpaWifiStation::accessPointsFound, [ctl](const QList<WpaWifiStation::AccessPoint> &aps){
		qDebug() << "accessPointsFound" << aps.size();
		for (const auto &x : aps) {
			qDebug() << "BSSID:" << x.m.BSSID << "\tslvl:" << x.m.signalLvl << "\tconnected:" << x.m.connected << "\tSSID:" << x.m.SSID;
		}
		qDebug() << "";
	});
	QObject::connect(ctl, &WpaWifiStation::connected, [ctl](){
		qDebug() << "connected";
	});
	QObject::connect(ctl, &WpaWifiStation::disconnected, [ctl](){
		qDebug() << "disconnected";
	});
	QObject::connect(ctl, &WpaWifiStation::connectError, [ctl](){
		qDebug() << "connectError";
	});

	return app.exec();
}

