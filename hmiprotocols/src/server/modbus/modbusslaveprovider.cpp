#include "modbus/modbusslaveprovider.h"
#include "extmodbusserver.h"
#include "typeconverter.h"
#include <QDebug>
#include <QUrl>

#define scast(t,v) static_cast<t>(v)


void ModbusSlaveProvider::insertRegister(const MbRegister &reg)
{
	logDebug() << "insertRegister " << reg.type << reg.address << reg.length;
	// server is busy while updating
	this->mbDev->setValue(QModbusServer::DeviceBusy, 0xffff);
	this->mbDev->insertRegister(TypeConverter::conv(reg.type), reg.address, reg.length);
}


void ModbusSlaveProvider::updateDatabase(const MbRegister &reg, const QVariant &data)
{
	logDebug() << "updateServerDatabase " << reg.address << data;
	HMIVariant::Array array;
	bool ok = HMIVariant::QVariantToArray(data, array, reg.length);
	if (ok) {
		ok = this->mbDev->setData( {TypeConverter::conv(reg.type), reg.address, array}, false );
	} else {
		logDebug() << "updateDatabase failed" << reg.type << reg.address << array;
	}
}


bool ModbusSlaveProvider::pause()
{
	if (mbDev) {
		switch (mbDev->state()) {
			case QModbusDevice::ConnectingState:
			case QModbusDevice::ConnectedState: {
				logDebug() << "pause";
				mbDev->disconnectDevice();
				return true;
			} break;
			default: { /* NOP */ } break;
		}
	}
	return false;
}


bool ModbusSlaveProvider::resume()
{
	if (mbDev) {
		switch (mbDev->state()) {
			case QModbusDevice::UnconnectedState: {
				logDebug() << "resume";
				return mbDev->connectDevice();
			} break;
			default: { /* NOP */ } break;
		}
	}
	return false;
}


HMIVariant::Array ModbusSlaveProvider::getData(const MbRegister &reg)
{
	QModbusDataUnit unit {TypeConverter::conv(reg.type), reg.address, reg.length};
	bool ok = this->mbDev->data(&unit);
	return (ok)? unit.values() : HMIVariant::Array(0);
}


void ModbusSlaveProvider::initBase()
{
	if (mbDev && config) {
        mbDev->provider = this;

		// todo delete qt 14.2+
        connect(mbDev->qDev(), &QModbusServer::stateChanged, [this](int state){
			logDebug() << "stateChanged " << state;
			switch (scast(QModbusDevice::State, state)) {
                case QModbusDevice::UnconnectedState: {
                    emit clientConnected(0);
                } break;
                case QModbusDevice::ConnectedState: {
					emit clientDisconnected(0);
                } break;
                case QModbusDevice::ConnectingState: {} break;
				case QModbusDevice::ClosingState: {} break;
			}
		});
        connect(mbDev->qDev(), &QModbusServer::errorOccurred, [this](int error){
			logWarn() << "errorOccurred: " << mbDev->errorString();
			switch (scast(QModbusDevice::Error, error)) {
				case QModbusDevice::NoError: {} break;
				case QModbusDevice::ReadError: {} break;
				case QModbusDevice::WriteError: {} break;
				case QModbusDevice::ConnectionError: {} break;
				case QModbusDevice::ConfigurationError: {} break;
				case QModbusDevice::TimeoutError: {} break;
				case QModbusDevice::ProtocolError: {} break;
				case QModbusDevice::ReplyAbortedError: {} break;
				case QModbusDevice::UnknownError: {} break;
			}
		});
	} else {
		lFatal("ModbusSlaveProvider::initBase must be called after mbDev && config allocation");
	}
}


void ModbusSlaveProvider::onRestartFillConfigCmn(Config *cfg)
{
    if (!cfg->addr) {
        cfg->addr = 1;
    }
}


ModbusSlaveProvider::ModbusSlaveProvider(QObject *parent)
		: ProtocolServerProvider(parent)
{
	mbDev = nullptr;
	config = nullptr;
}


ModbusSlaveProvider::~ModbusSlaveProvider()
{
	qDebug() << "~ModbusSlaveProvider";
	if (mbDev) {
		mbDev->disconnectDevice();
	}
	delete config;
}


void ExtModbusServer::pDataWritten(QModbusDataUnit::RegisterType regType, int address, int size)
{
	ModbusSlaveProvider *provider = qobject_cast<ModbusSlaveProvider *>(this->provider);
	if (provider) {
		lDebug(provider) << "handleData " << regType << address << size;
		emit provider->dataUpdated( {TypeConverter::conv(regType), scast(quint16, address), scast(quint16, size)} );
	}
}
