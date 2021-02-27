#include "ssdpserver.h"
#include <QUdpSocket>
#include <QNetworkInterface>

#define toC() toStdString().c_str()


using namespace SSDP;


void SSDPServer::onReadyRead()
{
	QUdpSocket *sock = qobject_cast<QUdpSocket*>(sender());
	while (sock->hasPendingDatagrams()) {
		QByteArray datagram;
		QHostAddress addr;
		quint16 port;
		datagram.resize(sock->pendingDatagramSize());
		sock->readDatagram(datagram.data(), datagram.size(), &addr, &port);

		QString request = datagram;
		if (
			request.contains(QStringLiteral("REQUEST;")) &&
			(request.contains(QStringLiteral("CLASS=3;")) ||
			request.contains(QStringLiteral("CLASS=0;")))
			// todo TYPE=XXX; \\  TYPE=0;
		) {
			char response[512];
			sprintf(response,
					"REQUEST_ANS;"
					"STATUS=%u;"
					"NAME=%s;"
					"VERSION=%s;"
					"IDENT=%s;"
					"TYPE=%s;"
					"EXT=%s;",
					p.status.b,
					p.name.toC(),
					p.version.toC(),
					p.ident.toC(),
					p.type.toC(),
					p.extdata.toC());
			datagram = response;
			if ( sock->writeDatagram(datagram.data(), datagram.size(), addr, port) )
				emit requestReceived();
		}
		if (request.contains(QStringLiteral("WINK;"))) {
			datagram = "WINK_ANS;RESULT=0;";
			if ( sock->writeDatagram(datagram.data(), datagram.size(), addr, port) )
				emit winkReceived();
		}
	}
}


void SSDPServer::restart()
{
	QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
	if (ifaces.size()) {
		for (auto &s : sockets) {
			s->close();
			s->deleteLater();
		}
		sockets.clear();
		sockets.reserve(ifaces.size());
		for (auto &i : ifaces) {
			if (i.type() != QNetworkInterface::Loopback && (i.flags() & QNetworkInterface::CanMulticast)) {
				sockets.push_back(new QUdpSocket(this));
				QUdpSocket &sock = *sockets.last();
				sock.bind(SERVER_PORT, QUdpSocket::ShareAddress);
				bool ok = sock.joinMulticastGroup(QHostAddress(SERVER_GROUP), i);
				if (ok) {
					connect(&sock, &QUdpSocket::readyRead, this, &SSDPServer::onReadyRead);
				}
			}
		}
	}
}


void SSDPServer::updateParams(const Params &prm)
{
	p = prm;
}


SSDPServer::SSDPServer(QObject *parent) : QObject(parent)
{}


SSDPServer::~SSDPServer()
{
	qDebug() << "~SSDPServer";
}
