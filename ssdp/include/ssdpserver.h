#ifndef SSDPSERVER_H
#define SSDPSERVER_H

#include <QObject>
#include <QList>

#include "ssdp.h"

class QUdpSocket;

class SSDPServer : public QObject
{
	Q_OBJECT
public:
	SSDP::Params p;

	SSDPServer(QObject *parent = nullptr);
	virtual ~SSDPServer();

public slots:
	void restart();
	void updateParams(const SSDP::Params &prm);

signals:
	void requestReceived();
	void winkReceived();

private slots:
	void onReadyRead();

private:
	QList <QUdpSocket*> sockets;
};

#endif // SSDPSERVER_H
