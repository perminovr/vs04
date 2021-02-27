#ifndef ExtModbusRtuClient_H
#define ExtModbusRtuClient_H

#include <QModbusRtuSerialMaster>
#include "extmodbusclient.h"

class ExtModbusRtuClient : public ExtModbusClient
{
	Q_OBJECT
public:
	ExtModbusRtuClient(QObject *parent = nullptr) : ExtModbusClient(parent) {
		self = new QModbusRtuSerialMaster(this);
		initBase();
	}
	virtual ~ExtModbusRtuClient() = default;
};

#endif // ExtModbusRtuClient_H
