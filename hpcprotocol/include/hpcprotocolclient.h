#ifndef HPCPCLIENT_H
#define HPCPCLIENT_H

#include "hpcprotocol.h"
#include <QTcpSocket>


class HPCProtocolClient : public HPCProtocol
{
	Q_OBJECT
public:
	void connect(const QString &serverIp, quint16 port);
	void disconnect();
	bool isConnected();

	HPCProtocolClient(QObject *parent = nullptr);
	virtual ~HPCProtocolClient();

public slots:
	inline void cmdUpdate() { this->ctlSend(IdCmd::cUpd, 0); }
	inline void cmdRequestAll() { this->ctlSend(IdCmd::cReqAll, 0); }
	inline void cmdRequestSysLog() { this->ctlSend(IdCmd::cReqSysLog, 0); }
	inline void cmdRequestUserLog() { this->ctlSend(IdCmd::cReqUserLog, 0); }
	inline void cmdRequestConfig() { this->ctlSend(IdCmd::cReqConfig, 0); }
	inline void cmdEnterPrivil() { this->ctlSend(IdCmd::cEnterPrivil, 0); }
	inline void cmdLeavePrivil() { this->ctlSend(IdCmd::cLeavePrivil, 0); }
	inline void cmdOpenRoot() { this->ctlSend(IdCmd::cOpenRoot, 0); }
	inline void cmdSysReboot() { this->ctlSend(IdCmd::cSysReboot, 0); }
	inline void cmdSysUpdate() { this->ctlSend(IdCmd::cSysUpdate, 0); }

signals:

private:
	QTcpSocket *socket;
	bool m_connected;
};

#endif // HPCPCLIENT_H
