#include "protocolctlhandler.h"
#include <sys/socket.h>


enum ProtocolCtlType {
    pct_nDef,
    pct_short,
    pct_full,
	pct_event,
	pct_cmd
};


void ProtocolCtlHandler::add(ProtocolCtl *pc)
{
	this->protocols.insert(pc->m.si.signature, pc);
}


void ProtocolCtlHandler::del(ProtocolCtl *pc)
{
	this->protocols.remove(pc->m.si.signature);
}


void ProtocolCtlHandler::read()
{
	QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
	if (client && client == socket) {
		while (client->bytesAvailable()) {
			// QByteArray frames = client->readAll();
			// int fssize = frames.size();
			// for (;fssize >= FrameHeader::size;) {
			// 	FrameHeader header(frames);
			// 	switch (header.reason) {
			// 	default: {
			// 		fssize = 0; // error => stop handling
			// 	} break;
			// 	}
			// 	frames = frames.right(fssize);
			// }
		}
	}

}


bool ProtocolCtlHandler::restart(QTcpSocket *socket)
{
	if (socket) {
		this->close();
		this->socket = socket;

		// send RST on disconnect instead FIN ( => nowait )
		auto fd = socket->socketDescriptor();
		struct linger lin = {
			.l_onoff = 1,
			.l_linger = 0
		};
		::setsockopt((int)fd, SOL_SOCKET, SO_LINGER, (const char *)&lin, sizeof(struct linger));

		connect(socket, &QTcpSocket::readyRead, this, &ProtocolCtlHandler::read);
		connect(socket, &QTcpSocket::disconnected, this, &ProtocolCtlHandler::close);
		return true;
	}
	return false;
}


void ProtocolCtlHandler::close()
{
	if (socket) {
		socket->close();
		socket->deleteLater();
		socket = nullptr;
		emit disconnected();
	}
}

ProtocolCtlHandler::ProtocolCtlHandler(QObject *parent) : QObject(parent)
{
	socket = nullptr;
}


ProtocolCtlHandler::~ProtocolCtlHandler()
{
	qDebug() << "~ProtocolCtlHandler";
	// del socket? todo
}
