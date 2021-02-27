#include "hpcprotocolclient.h"
#include <QHostAddress>


void HPCProtocolClient::connect(const QString &serverIp, quint16 port)
{
	this->disconnect();
	socket = new QTcpSocket(this);
	QObject::connect(socket, &QTcpSocket::connected, [this](){
		m_connected = true;
		HPCProtocol::restart(socket);
		emit this->connected();
	});
	socket->connectToHost(QHostAddress(serverIp), port);
}


void HPCProtocolClient::disconnect()
{
	if (socket) {
		HPCProtocol::close();
		socket = nullptr;
		m_connected = false;
	}
}


HPCProtocolClient::HPCProtocolClient(QObject *parent)
	: HPCProtocol(parent)
{
	socket = nullptr;
	m_connected = false;
}


HPCProtocolClient::~HPCProtocolClient()
{}


bool HPCProtocolClient::isConnected()
{
	return m_connected;
}
