#include "hpcprotocolserver.h"

#include <QNetworkDatagram>

#define PRIVILEGED_MODE(cmd, ...) { \
	if (this->isPrivileged()) { \
		__VA_ARGS__ \
	} else { \
		emit this->privilegeError(cmd); \
	} \
}


void HPCProtocolServer::handleControl(IdCmd cmd, const QByteArray &prm)
{
	Q_UNUSED(prm);

	switch (cmd) {
	case cEcho: {
		/* already done */
	} break;
	case cReqAll: {
		setPanelIdent(panelIdent());
		setPanelEth(panelEth());
		setPanelState(panelState());
		setPanelWifi(panelWifi());
		setPanelShort(panelShort());
        setPanelUserPassword(panelUserPassword());
	} break;
	case cUpd: {
		emit this->cmdUpdateReceived();
	} break;
	case cReqSysLog: {
		emit this->fullSysLogRequested();
	} break;
	case cReqUserLog: {
		emit this->fullUserLogRequested();
	} break;
	case cReqConfig: {
		emit this->configRequested();
	} break;
	case cEnterPrivil: {
		this->privilegedMode(true);
	} break;
	case cLeavePrivil: {
		this->privilegedMode(false);
	} break;
	case cOpenRoot: {
		PRIVILEGED_MODE(cmd, emit this->cmdOpenRootReceived(); )
	} break;
	case cSysReboot: {
		PRIVILEGED_MODE(cmd, emit this->cmdRebootReceived(); )
	} break;
	case cSysUpdate: {
		PRIVILEGED_MODE(cmd, emit this->cmdSysUpdReceived(); )
	} break;
	default: {
		/* NOP */
	} break;
	}
}


void HPCProtocolServer::privilegedMode(bool status)
{
	this->setIsPrivileged(status);
	emit this->privilegedModeChanged(status);
}


bool HPCProtocolServer::restart(const QString &serverIp, quint16 port)
{
	bool ret = false;

	if (tcpServer) {
		HPCProtocol::close();
		tcpServer->close();
		tcpServer->deleteLater();
	}
	tcpServer = new QTcpServer(this);
	ret = tcpServer->listen(QHostAddress(serverIp), port);
	if (ret) {
		tcpServer->setMaxPendingConnections(2);
		connect(tcpServer, &QTcpServer::newConnection, this, &HPCProtocolServer::handleConnection);
	}

	return ret;
}


HPCProtocolServer::HPCProtocolServer(QObject *parent)
	: HPCProtocol(parent)
{
	tcpServer = nullptr;
	connect(this, &HPCProtocol::ctlReceived, this, &HPCProtocolServer::handleControl);
}


HPCProtocolServer::~HPCProtocolServer()
{}


void HPCProtocolServer::handleConnection()
{
	QTcpSocket *socket = tcpServer->nextPendingConnection();
	HPCProtocol::restart(socket);
	emit this->connected();
}
