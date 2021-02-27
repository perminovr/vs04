#ifndef WIFICTLWORKER_H
#define WIFICTLWORKER_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QQueue>

#include "wpa_ctrl.h"
#include "wpawifistation.h"

template <class T>
class UniQue : public QQueue<T> {
public:
    inline void enqueue_unique(const T &t) {
		if (!QList<T>::contains(t)) { QList<T>::append(t); }
		else { // reque
			QList<T>::removeOne(t);
			QList<T>::append(t);
		}
	}
};

class WifiCtlWorker : public QObject {
    Q_OBJECT
public:

	enum State {
		Idle,
		StartingProcess,
		Initialization,
		EventsWaiting,
		Scanning,
		ScanningResult,
		Connecting,
		Disconnecting
	};

	enum QueCommand_e {
		NDEF,
		SCAN,
		CONNECT,
		DISCONNECT
	};

    WifiCtlWorker(QObject *parent = nullptr);
    virtual ~WifiCtlWorker();

public slots:
    void setIsScanning(bool val);

	void setIface(const QString &iface) { this->iface = iface; }
    void startWork();
    void stopWork();

    void connectTo(const IWifiStation::AccessPoint &targetAp);
    void disconnectFromAp();

signals:
	void accessPointsFound(const QList<IWifiStation::AccessPoint> &aps);
	void started();
	void stopped();
	void wpa_started();
	void wpa_stopped();
	void ap_connected();
	void ap_disconnected();
	void ap_connectError();

/* private */ signals:
	void p_nextJob();

private slots:
    void doJob();
	void onProcErrorOccurred(QProcess::ProcessError error);
	void onProcFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onProcStarted();

public:
	struct QueCommand {
		QueCommand_e self;
		int timeout;
		QueCommand() : self(NDEF), timeout(0) { ; }
		QueCommand(QueCommand_e s, int t) : self(s), timeout(t) { ; }
		QueCommand(const QueCommand &c) { self=(c.self); timeout=(c.timeout); }
		void operator=(const QueCommand &c) { self=(c.self); timeout=(c.timeout); }
		bool operator==(const QueCommand &c) { return (self==(c.self)); }
	};

private:
	QString iface;
	QProcess *process;
    struct wpa_ctrl *ctrl;
	char *ctrlbuf;
    bool running;
    State state;
	UniQue<QueCommand> cque;
	QueCommand lastCmd;
	QTimer *cmdFailedTimer;

	QTimer *scanTimer;
	QList<IWifiStation::AccessPoint> aps;

	IWifiStation::AccessPoint targetAp;

	void prepForCmd(const QueCommand &cmd);
	void endCmd();

	void stop();
    void ctrlFlush();
	char *ctrlReceive();
	void setState(State state);
};

#endif // WIFICTLWORKER_H
