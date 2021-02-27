#include "wifictlworker.h"
#include <QDebug>
#include <QThread>

#define scast(t,v) static_cast<t>(v)
#define QueuedUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::QueuedConnection)))

/*
https://w1.fi/wpa_supplicant/devel/ctrl_iface_page.html
*/

#define SLEEP_BETWEEN_STATES_MS	(25)
#define WPA_SUPPLICANT_EXEC 	"wpa_supplicant"
#define WPA_SUPPLICANT_CTL 		"/var/run/wpa_supplicant"
#define WPA_SUPPLICANT_DRV		"nl80211"
#define WPA_SUPPLICANT_BUF_SZ 	2048
#define WPA_SUPPLICANT_SCAN_PERIOD		7000
#define WPA_SUPPLICANT_CONNECT_TO		10000
#define WPA_SUPPLICANT_DISCONNECT_TO	1000


static inline QString setNetwork_unq(const char *var, const char *val)
{
	return QString("SET_NETWORK 0 %1 %2").arg(var).arg(val);
}


static inline QString setNetwork_q(const char *var, const char *val)
{
	return QString("SET_NETWORK 0 %1 \"%2\"").arg(var).arg(val);
}


#define setStateAndRet(s) \
	{setState(s); return;}
#define endCmdAndRet(s) \
	{endCmd(); return;}


void WifiCtlWorker::doJob()
{
	usleep(SLEEP_BETWEEN_STATES_MS * 1000);
	if (running) {
		switch (state) {
		case State::Idle: {
			// nothing to do
		} break;
		case State::StartingProcess: {
			this->process = new QProcess(this);
			QStringList args;
			args << QStringLiteral("-D" WPA_SUPPLICANT_DRV); // driver
			args << QStringLiteral("-C" WPA_SUPPLICANT_CTL); // control object
			args << QStringLiteral("-i") + this->iface; // iface
			process->setProgram(QStringLiteral(WPA_SUPPLICANT_EXEC));
			process->setArguments(args);
			process->setStandardOutputFile(QProcess::nullDevice());
			process->setStandardErrorFile(QProcess::nullDevice());
			connect(process, &QProcess::errorOccurred, this, &WifiCtlWorker::onProcErrorOccurred, QueuedUniqueConnection);
			connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &WifiCtlWorker::onProcFinished, QueuedUniqueConnection);
			connect(process, &QProcess::started, this, &WifiCtlWorker::onProcStarted, QueuedUniqueConnection);
			process->start(); // isn't detached
			setStateAndRet(State::Idle); // waiting for process
		} break;
		case State::Initialization: {
			QString ctl = QString(WPA_SUPPLICANT_CTL "/%1").arg(iface);
			ctrl = wpa_ctrl_open(ctl.toLocal8Bit().data());
			if (ctrl) {
				wpa_ctrl_attach(ctrl); // sub for events
				setStateAndRet(State::EventsWaiting);
			}
			setStateAndRet(state); // waiting for open success todo timeout?
		} break;
		case State::EventsWaiting: {
			// wpa events
			{
				char *msg = ctrlReceive();
				if (msg) {
					if (strstr(msg, "CTRL-EVENT-CONNECTED") != 0) {
						targetAp.m.connected = true;
						emit ap_connected();
					} else
					if (strstr(msg, "CTRL-EVENT-DISCONNECTED") != 0) {
						targetAp.m.connected = false;
						emit ap_disconnected();
					}
				}
			}
			// driver events
			if (state == State::EventsWaiting) {
				if (!cque.empty()) {
					auto cmd = cque.dequeue();
					if (cmd.self != QueCommand_e::NDEF) {
						prepForCmd(cmd);
						ctrlFlush();
						switch (cmd.self) {
							case QueCommand_e::SCAN: {
								wpa_ctrl_request2(ctrl, "SCAN");
								setStateAndRet(State::Scanning);
							} break;
							case QueCommand_e::CONNECT: {
								QString tmp;

								// using network with id = 0 only
								wpa_ctrl_request2(ctrl, "DISCONNECT");
								wpa_ctrl_request2(ctrl, "DISABLE_NETWORK 0");
								wpa_ctrl_request2(ctrl, "REMOVE_NETWORK 0");
								wpa_ctrl_request2(ctrl, "ADD_NETWORK");

								tmp = setNetwork_unq("scan_ssid", "1"); // working with hidden ap
								wpa_ctrl_request2(ctrl, tmp.toLocal8Bit().data());

								tmp = (strlen(targetAp.m.BSSID) > 0)? // apn/bssid
									setNetwork_unq("bssid", targetAp.m.BSSID) :
									setNetwork_q("ssid", targetAp.m.SSID);
								wpa_ctrl_request2(ctrl, tmp.toLocal8Bit().data());

								if (strlen(targetAp.m.uname) > 0) { // EAP
									tmp = setNetwork_unq("key_mgmt", "WPA-EAP");
									wpa_ctrl_request2(ctrl, tmp.toLocal8Bit().data());
									tmp = setNetwork_q("identity", targetAp.m.uname);
									wpa_ctrl_request2(ctrl, tmp.toLocal8Bit().data());
									tmp = setNetwork_q("password", targetAp.m.password);
									wpa_ctrl_request2(ctrl, tmp.toLocal8Bit().data());
								} else if (strlen(targetAp.m.password) > 0) { // PSK
									tmp = setNetwork_unq("key_mgmt", "WPA-PSK");
									wpa_ctrl_request2(ctrl, tmp.toLocal8Bit().data());
									tmp = setNetwork_q("psk", targetAp.m.password);
									wpa_ctrl_request2(ctrl, tmp.toLocal8Bit().data());
								} else { // NONE
									tmp = setNetwork_unq("key_mgmt", "NONE");
									wpa_ctrl_request2(ctrl, tmp.toLocal8Bit().data());
								}

								wpa_ctrl_request2(ctrl, "ENABLE_NETWORK 0");
								wpa_ctrl_request2(ctrl, "RECONNECT");
								setStateAndRet(State::Connecting);
							} break;
							case QueCommand_e::DISCONNECT: {
								wpa_ctrl_request2(ctrl, "DISCONNECT");
								setStateAndRet(State::Disconnecting);
							} break;
							default: break;
						}
					}
				}
			}
			setStateAndRet(state);
		} break;
		case State::Scanning: {
			char *msg = ctrlReceive();
			if (msg) {
				if (strstr(msg, "CTRL-EVENT-SCAN-RESULTS") != 0) {
					wpa_ctrl_request2(ctrl, "SCAN_RESULTS");
					setStateAndRet(State::ScanningResult);
				}
			}
			setStateAndRet(state);
		} break;
		case State::ScanningResult: {
			char *msg = ctrlReceive();
			if (msg) {
				if (strstr(msg, "bssid / frequency / signal level / flags / ssid") != 0) {
					QStringList qmsg = QString(msg).split('\n');
					qmsg.removeFirst(); // bssid / f...
					qmsg.removeLast(); // ""
					QList<IWifiStation::AccessPoint> aps;
					aps.reserve(qmsg.size());
					for (auto &x : qmsg) {
						IWifiStation::AccessPoint ap;
						QStringList record = x.split('\t');
						if (record.size() == 5) {
							strcpy(ap.m.BSSID, record[0].toLocal8Bit().data());
							int frequency = record[1].toInt();
							ap.m.frequency = (frequency < 3500)? IWifiStation::Frequency::FREQ_2_4_GHZ :
									(frequency < 10000)? IWifiStation::Frequency::FREQ_5_GHZ :
									IWifiStation::Frequency::FREQ_60_GHZ;
							ap.m.signalLvl = record[2].toInt();
							{
								ap.m.security = IWifiStation::Security::None;
								QStringList flags = record[3].split("][");
								int sz = flags.size();
								if (sz > 0) {
									/*if (sz >= 2) {
										flags.first().remove(0, 1);
										flags.last().remove(flags.last().length()-1, 1);
									} else {
										flags.first().remove(0, 1);
										flags.first().remove(flags.last().length()-1, 1);
									}*/
									for (const auto &f : flags) {
										if (f.contains("WPA")) {
											if (f.contains("PSK")) ap.m.security = IWifiStation::Security::WPA_PSK;
											else if (f.contains("EAP")) ap.m.security = IWifiStation::Security::WPA_EAP;
										}
									}
								}
							}
							strcpy(ap.m.SSID, record[4].toLocal8Bit().data());
							if (targetAp.m.connected) {
								ap.m.connected = (strcmp(targetAp.m.SSID, ap.m.SSID) == 0) || (strcmp(targetAp.m.BSSID, ap.m.BSSID) == 0);
							}
							aps.push_back(ap);
						}
					}
					if (aps.size())
						emit accessPointsFound(aps);
					endCmdAndRet();
				}
			}
			setStateAndRet(state);
		} break;
		case State::Connecting: {
			char *msg = ctrlReceive();
			if (msg) {
				if (strstr(msg, "CTRL-EVENT-CONNECTED") != 0) {
					targetAp.m.connected = true;
					emit ap_connected();
					endCmdAndRet();
				} else
				if (strstr(msg, "CTRL-EVENT-DISCONNECTED") != 0) {
					targetAp.m.connected = false;
					emit ap_connectError();
					endCmdAndRet();
				}
			}
			setStateAndRet(state);
		} break;
		case State::Disconnecting: {
			char *msg = ctrlReceive();
			if (msg) {
				if (strstr(msg, "CTRL-EVENT-DISCONNECTED") != 0) {
					targetAp.m.connected = false;
					emit ap_disconnected();
					endCmdAndRet();
				}
			}
			setStateAndRet(state);
		} break;
		}
	}
}

void WifiCtlWorker::prepForCmd(const QueCommand &cmd)
{
	lastCmd = cmd;
	cmdFailedTimer->start(cmd.timeout);
}


void WifiCtlWorker::endCmd()
{
	lastCmd.self = QueCommand_e::NDEF;
	lastCmd.timeout = 0;
	cmdFailedTimer->stop();
	setState(State::EventsWaiting);
}


void WifiCtlWorker::connectTo(const IWifiStation::AccessPoint &ap)
{
	if (running) {
		targetAp = ap;
		targetAp.m.connected = false;
		cque.enqueue_unique({QueCommand_e::CONNECT, WPA_SUPPLICANT_CONNECT_TO});
	}
}


void WifiCtlWorker::disconnectFromAp()
{
	if (running) {
		cque.enqueue_unique({QueCommand_e::DISCONNECT, WPA_SUPPLICANT_DISCONNECT_TO});
	}
}


void WifiCtlWorker::stop()
{
	if (running) {
		setState(State::Idle);
		running = false;
		scanTimer->stop();
		cmdFailedTimer->stop();
		if (ctrl) wpa_ctrl_close(ctrl);
		process->kill();
		emit stopped();
	}
}


void WifiCtlWorker::stopWork()
{
	stop();
}


void WifiCtlWorker::startWork()
{
	if (!running) {
		system("killall -9 " WPA_SUPPLICANT_EXEC);
		setState(State::StartingProcess);
		running = true;
		emit started();
	}
}


char *WifiCtlWorker::ctrlReceive()
{
	if (ctrl) {
		size_t rlen = WPA_SUPPLICANT_BUF_SZ-1;
		int res = wpa_ctrl_recv(ctrl, ctrlbuf, &rlen);
		if (res == 0 && rlen > 0) {
			ctrlbuf[rlen] = 0;
			return ctrlbuf;
		}
	}
	return nullptr;
}


void WifiCtlWorker::ctrlFlush()
{
	for (;;) {
		if (ctrlReceive() == nullptr) return;
	}
}


void WifiCtlWorker::onProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	Q_UNUSED(exitCode)
	Q_UNUSED(exitStatus)
	QProcess *proc = qobject_cast<QProcess *>(sender());
	if (proc) {
		// wanna to restart if crashed
		if (this->running) {
			this->setState(State::Idle);
			proc->start();
		}
		// user finished process -> delete
		else {
			proc->deleteLater();
			emit wpa_stopped();
		}
	}
}


void WifiCtlWorker::onProcStarted()
{
	QProcess *proc = qobject_cast<QProcess *>(sender());
	if (proc) {
		this->setState(State::Initialization);
		emit wpa_stopped();
	}
}


void WifiCtlWorker::onProcErrorOccurred(QProcess::ProcessError error)
{
	QProcess *proc = qobject_cast<QProcess *>(sender());
	if (proc) {
		switch (error) {
			case QProcess::FailedToStart:
				break;
			case QProcess::Crashed:
				break;
			default:
				break;
		}
	}
}


void WifiCtlWorker::setState(State state)
{
	this->state = state;
	emit p_nextJob();
}


void WifiCtlWorker::setIsScanning(bool val)
{
    (val)? scanTimer->start(0) : scanTimer->stop();
}


WifiCtlWorker::WifiCtlWorker(QObject *parent) : QObject(parent)
{
	process = nullptr;
	running = false;
	state = State::Idle;
	ctrl = nullptr;
	ctrlbuf = new char[WPA_SUPPLICANT_BUF_SZ];

	connect(this, &WifiCtlWorker::p_nextJob, this, &WifiCtlWorker::doJob, QueuedUniqueConnection);

	cmdFailedTimer = new QTimer(this);
	cmdFailedTimer->setSingleShot(true);
	connect(cmdFailedTimer, &QTimer::timeout, this, [this](){
		if (this->running) {
			this->setState(State::EventsWaiting);
			switch (this->lastCmd.self) {
				case QueCommand_e::CONNECT:
					//this->cque.enqueue_unique({QueCommand_e::CONNECT, WPA_SUPPLICANT_CONNECT_TO}); // try again
					emit ap_connectError();
					break;
				default: break;
			}
			endCmd();
		}
	}, QueuedUniqueConnection);

	scanTimer = new QTimer(this);
	scanTimer->setSingleShot(true);
	connect(scanTimer, &QTimer::timeout, this, [this](){
		if (this->running) {
			cque.enqueue_unique({QueCommand_e::SCAN, (WPA_SUPPLICANT_SCAN_PERIOD-500)});
			this->scanTimer->start(WPA_SUPPLICANT_SCAN_PERIOD);
			return;
		}
		this->scanTimer->start(WPA_SUPPLICANT_SCAN_PERIOD/10);
	}, QueuedUniqueConnection);
}


WifiCtlWorker::~WifiCtlWorker()
{
	stop();
	delete ctrlbuf;
}



void WpaWifiStation::connectToAp(const IWifiStation::AccessPoint &ap)
{
	this->currentAp = ap;
	this->currentAp.m.connected = false;
	// should be called in connection order
	emit p_disconnectFromAp();
	emit p_connectTo(this->currentAp);
}


void WpaWifiStation::disconnectFromAp()
{
	emit p_disconnectFromAp();
}


void WpaWifiStation::restart(const QString &iface)
{
	// should be called in connection order
	emit p_stopped();
	emit p_setIface(iface);
	emit p_started();
}


void WpaWifiStation::stop()
{
	emit p_stopped();
}


WpaWifiStation::WpaWifiStation(QObject *parent) : IWifiStation(parent)
{
	QThread *thread = new QThread();
	worker = new WifiCtlWorker();

	// connect order is important
	connect(this, &WpaWifiStation::p_stopped, worker, &WifiCtlWorker::stopWork, QueuedUniqueConnection);
	connect(this, &WpaWifiStation::p_setIface, worker, &WifiCtlWorker::setIface, QueuedUniqueConnection);
	connect(this, &WpaWifiStation::p_started, worker, &WifiCtlWorker::startWork, QueuedUniqueConnection);
	connect(this, &WpaWifiStation::p_disconnectFromAp, worker, &WifiCtlWorker::disconnectFromAp, QueuedUniqueConnection);
	connect(this, &WpaWifiStation::p_connectTo, worker, &WifiCtlWorker::connectTo, QueuedUniqueConnection);
    connect(this, &WpaWifiStation::isScanningChanged, worker, &WifiCtlWorker::setIsScanning, QueuedUniqueConnection);
	//
	connect(worker, &WifiCtlWorker::accessPointsFound, this, &IWifiStation::accessPointsFound, QueuedUniqueConnection);
	connect(worker, &WifiCtlWorker::started, this, &IWifiStation::started, QueuedUniqueConnection);
	connect(worker, &WifiCtlWorker::stopped, this, &IWifiStation::stopped, QueuedUniqueConnection);
	connect(worker, &WifiCtlWorker::ap_disconnected, this, [this](){
		this->currentAp.m.connected = false;
		emit this->disconnected();
	});
	connect(worker, &WifiCtlWorker::ap_connectError, this, [this](){
		this->currentAp.m.connected = false;
		emit this->connectError();
	});
	connect(worker, &WifiCtlWorker::ap_connected, this, [this](){
		this->currentAp.m.connected = true;
		emit this->connected();
	});

	worker->moveToThread(thread);
    thread->start();
}


WpaWifiStation::~WpaWifiStation()
{
	QThread *thread;
	thread = worker->thread();
	worker->stopWork();
	thread->quit();
	thread->wait();
	delete thread;
	delete worker;
}
