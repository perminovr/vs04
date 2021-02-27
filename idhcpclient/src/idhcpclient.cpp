#include "idhcpclient.h"
#include "pipeserver.h"
#include "pipeclient.h"
#include <QDataStream>


#ifndef DHCPNOTIFY_PIPE_NAME
# define DHCPNOTIFY_PIPE_NAME		"/tmp/dhcpnotify_in"
#endif


void IDhcpClient::notify(const QString &iface, const QString &ip, const QString &mask, const QString &defGateway, bool add)
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_3);

	out << quint16(0) << iface << ip << mask << defGateway << add;
	out.device()->seek(0);
	out << quint16(data.size() - sizeof(quint16));

	this->client->write(data);
}


void IDhcpClient::onClientData(const QByteArray &data)
{
	QDataStream in(data);
	in.setVersion(QDataStream::Qt_5_3);

	quint16 blockSize;
	QString iface;
	QString ip;
	QString netmask;
	QString defGateway;
	bool add;

	if (data.size() > static_cast<int>(sizeof(quint16))) {
		QNetworkAddressEntry addr;
		in >> blockSize >> iface >> ip >> netmask >> defGateway >> add;
		addr.setIp(QHostAddress(ip));
		addr.setNetmask(QHostAddress(netmask));
		emit notificationReceived(iface, addr, QHostAddress(defGateway), add);
	}
}


void IDhcpClient::start(Role role)
{
	bool ok;
	this->role = role;
	switch (role) {
		case Role::Implementer:
			this->server = new PipeServer(serverName, this);
			connect(this->server, &PipeServer::dataIsReady, this, &IDhcpClient::onClientData);
			ok = this->server->start(1);
			if (!ok)
				qCritical() << "Couldn't start dhcp (i)" <<  endl; // todo
			break;
		case Role::Notifier:
			this->client = new PipeClient(serverName, this);
			connect(this->client, &PipeClient::connected, this, &IDhcpClient::isReady);
			connect(this->client, &PipeClient::writeComplete, this, &IDhcpClient::complete);
			ok = this->client->start();
			if (!ok)
				qCritical() << "Couldn't start dhcp (n)" <<  endl;
			break;
	}
}


IDhcpClient::IDhcpClient(QObject *parent) : QObject(parent)
{
	this->serverName = QStringLiteral(DHCPNOTIFY_PIPE_NAME);
	this->client = nullptr;
	this->server = nullptr;
}


IDhcpClient::~IDhcpClient()
{}

