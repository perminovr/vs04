#include "pipeserver.h"

#include <QLocalSocket>


void PipeServer::readyRead()
{
	QLocalSocket *client = qobject_cast<QLocalSocket*>(sender());

	if (client) {
		if (client->bytesAvailable()) {
			emit dataIsReady( client->readAll() );
		}
	}
}


void PipeServer::handleConnection()
{
	QLocalSocket *clientConnection = server->nextPendingConnection();
	connect(clientConnection, &QLocalSocket::disconnected, clientConnection, &QLocalSocket::deleteLater);
	connect(clientConnection, &QLocalSocket::readyRead, this, &PipeServer::readyRead);
}


bool PipeServer::start(int maxConnections)
{
	if (server && !server->isListening()) {
		QLocalServer::removeServer(serverName);
		server->setMaxPendingConnections(maxConnections);
		if (server->listen(serverName)) {
			connect(server, &QLocalServer::newConnection, this, &PipeServer::handleConnection);
			return true;
		}
	}
	return false;
}


PipeServer::PipeServer(const QString &serverName, QObject *parent)
	: QObject(parent), serverName(serverName), server(new QLocalServer(this))
{}


PipeServer::~PipeServer()
{
	if (server && server->isListening()) {
		server->close();
	}
}
