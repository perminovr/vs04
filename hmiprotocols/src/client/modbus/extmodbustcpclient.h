#ifndef EXTMODBUSTCPSERVER_H
#define EXTMODBUSTCPSERVER_H

#include <QModbusTcpClient>
#include "extmodbusclient.h"

class ExtModbusTcpClient : public ExtModbusClient
{
	Q_OBJECT
public:	
	ExtModbusTcpClient(QObject *parent = nullptr) : ExtModbusClient(parent) {
		self = new QModbusTcpClient(this);
		initBase();
	}
	virtual ~ExtModbusTcpClient() = default;
};

#endif // EXTMODBUSTCPSERVER_H
