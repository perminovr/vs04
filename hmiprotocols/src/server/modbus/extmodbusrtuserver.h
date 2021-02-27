#ifndef EXTMODBUSRTUSERVER_H
#define EXTMODBUSRTUSERVER_H

#include <QModbusRtuSerialSlave>
#include "extmodbusserver.h"

class ExtModbusRtuServer : public QModbusRtuSerialSlave , public ExtModbusServer
{
public:
	ExtModbusRtuServer(QObject *parent = nullptr) : QModbusRtuSerialSlave(parent) {
		self = this;
	}
    virtual ~ExtModbusRtuServer() = default;

protected:
	virtual inline bool writeData(const QModbusDataUnit &unit) override { return ExtModbusServer::writeData(unit); }
	virtual inline bool readData(QModbusDataUnit *newData) const override { return ExtModbusServer::readData(newData); }
};

#endif // EXTMODBUSRTUSERVER_H
