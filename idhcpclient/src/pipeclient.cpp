#include "pipeclient.h"


bool PipeClient::write(const QByteArray &data)
{
	if (socket && socket->isOpen()) {
		qint64 res = socket->write(data);
		return res == data.size();
	}
	return false;
}


bool PipeClient::start()
{
	if (socket && !socket->isOpen()) {
		connect(socket, &QLocalSocket::connected, this, &PipeClient::connected);
		connect(socket, &QLocalSocket::bytesWritten, this, &PipeClient::onBytesWritten);
		socket->connectToServer(serverName, QIODevice::WriteOnly);
		return true;
	}
	return false;
}


void PipeClient::onBytesWritten(quint64 bytes)
{
	if (bytes > 0)
		emit writeComplete();
}


PipeClient::PipeClient(const QString &serverName, QObject *parent)
	: QObject(parent), serverName(serverName), socket(new QLocalSocket(this))
{}


PipeClient::~PipeClient()
{
	if (socket && socket->isOpen()) {
		socket->close();
	}
}
