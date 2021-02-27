#ifndef EXTMODBUSSERVER_H
#define EXTMODBUSSERVER_H

#include <map>
#include <QModbusServer>

typedef std::multimap <QModbusDataUnit::RegisterType, QModbusDataUnit> QModbusDataUnitMultiMap;

class ExtModbusServer
{
public:
	ExtModbusServer();
	virtual ~ExtModbusServer() = default;

	QObject *provider; //!< requiers for pDataWritten call (QModbusServer::dataWritten signal wrapper)
    QModbusServer *qDev() { return self; }

	void clearRegisterMap();
	void insertRegister(QModbusDataUnit::RegisterType key, quint16 address, quint16 length);
	bool setData(const QModbusDataUnit &newData, bool notify = true);
	bool writeData(const QModbusDataUnit &unit, bool notify);
	bool writeData(const QModbusDataUnit &unit);
    bool readData(QModbusDataUnit *newData) const;

	virtual bool connectDevice() { if (self) return self->connectDevice(); return false; }
	virtual void disconnectDevice() { if (self) self->disconnectDevice(); }
	virtual void setServerAddress(int serverAddress)  { if (self) self->setServerAddress(serverAddress); }
	virtual void setConnectionParameter(int parameter, const QVariant &value) { if (self) self->setConnectionParameter(parameter, value); }
	virtual bool setValue(int option, const QVariant &value) { if (self) return self->setValue(option, value); return false; }
	virtual bool data(QModbusDataUnit *newData) const { if (self) return self->data(newData); return false; }
	virtual bool setData(const QModbusDataUnit &unit) { if (self) return self->setData(unit); return false; }
	virtual QModbusDevice::State state() const { if (self) return self->state(); return QModbusDevice::State::UnconnectedState; }
	virtual QModbusDevice::Error error() const { if (self) return self->error(); return QModbusDevice::Error::UnknownError; }
	virtual QString errorString() const { if (self) return self->errorString(); return QString(); }

protected:
	QModbusServer *self;
	QModbusDataUnitMultiMap m_map;

	//!< must be implemented on provider side
	void pDataWritten(QModbusDataUnit::RegisterType regType, int address, int size);
};

#endif // EXTMODBUSSERVER_H
