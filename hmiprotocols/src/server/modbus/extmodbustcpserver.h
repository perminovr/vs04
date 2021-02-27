#ifndef EXTMODBUSTCPSERVER_H
#define EXTMODBUSTCPSERVER_H

#include <QModbusTcpServer>
#include "extmodbusserver.h"

class ExtModbusTcpServer : public QModbusTcpServer , public ExtModbusServer
{
	Q_OBJECT
public:	
	ExtModbusTcpServer(QObject *parent = nullptr) : QModbusTcpServer(parent) {
		self = this;
	}
    virtual ~ExtModbusTcpServer() = default;

protected:
	virtual inline bool writeData(const QModbusDataUnit &unit) override { return ExtModbusServer::writeData(unit); }
	virtual inline bool readData(QModbusDataUnit *newData) const override { return ExtModbusServer::readData(newData); }
};

#endif // EXTMODBUSTCPSERVER_H
