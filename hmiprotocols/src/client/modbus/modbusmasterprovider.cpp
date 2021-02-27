#include "modbus/modbusmasterprovider.h"
#include "extmodbusclient.h"
#include "typeconverter.h"
#include <QDebug>
#include <QUrl>

#define scast(t,v) static_cast<t>(v)
#define SplitMbReg(reg) TypeConverter::conv(reg.type),reg.address,reg.length


bool ModbusMasterProvider::devConnect()
{
    ProtocolClientProvider::clConnect();
    return mbDev->connectDevice();
}


void ModbusMasterProvider::devDisconnect()
{
    ProtocolClientProvider::clDisconnect();
    mbDev->disconnectDevice();
}


void ModbusMasterProvider::updateDatabase(const MbRegister &reg, const QVariant &data)
{
	if (mbDev) {
		HMIVariant::Array adata;
		HMIVariant::QVariantToArray(data, adata, reg.length);
        mbDev->setRegisterData(SplitMbReg(reg), adata);
	}
}


void ModbusMasterProvider::setPolledRegister(const MbRegister &reg, bool rdpolled, bool wrpolled)
{
	if (mbDev) {
		mbDev->setPolledRegister(SplitMbReg(reg), rdpolled, wrpolled);
	}
}


void ModbusMasterProvider::forceWriteAllDatabase()
{
    if (mbDev) {
        mbDev->setStartFromWr(true);
    }
}


bool ModbusMasterProvider::pause()
{
	if (mbDev) {
		switch (mbDev->state()) {
			case QModbusDevice::ConnectingState:
			case QModbusDevice::ConnectedState: {
                logDebug() << "pause";
                devDisconnect();
				return true;
			} break;
			default: { /* NOP */ } break;
		}
	}
	return false;
}


bool ModbusMasterProvider::resume()
{
	if (mbDev) {
		switch (mbDev->state()) {
			case QModbusDevice::UnconnectedState: {
                logDebug() << "resume";
                devConnect();
			} break;
			default: { /* NOP */ } break;
		}
	}
	return false;
}


void ModbusMasterProvider::initBase()
{
	if (mbDev && config) {
        QObject::connect(mbDev, &ExtModbusClient::stateChanged, [this](int state){
            switch (scast(QModbusDevice::State, state)) {
            case QModbusDevice::UnconnectedState:
            case QModbusDevice::ConnectedState:
                if (m_constate != state) {
                    m_constate = state;
                    logDebug() << "stateChanged " << state;
                    switch (scast(QModbusDevice::State, state)) {
                        case QModbusDevice::UnconnectedState: {
                            ProtocolClientProvider::onDisconnection();
                            emit connectedChange(false);
                        } break;
                        case QModbusDevice::ConnectedState: {
                            ProtocolClientProvider::onConnection();
                            emit connectedChange(true);
                        } break;
                        case QModbusDevice::ConnectingState: { } break;
                        case QModbusDevice::ClosingState: { } break;
                    }
                }
                break;
                default: break;
            }
		});
        QObject::connect(mbDev, &ExtModbusClient::errorOccurred, [this](int error){
            // log
            switch (scast(QModbusDevice::Error, error)) {
            case QModbusDevice::ConnectionError: {} break;
            default: logWarn() << "errorOccurred: " << mbDev->errorString(); break;
            }
            // handling
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
        QObject::connect(mbDev, &ExtModbusClient::errorData, [this](const QString &err, int code){
            logWarn() << "errorData: " << err << " code: " << code;
        });
        QObject::connect(mbDev, &ExtModbusClient::dataIsReady, [this](const QModbusDataUnit &unit){
			MbRegister reg;
			HMIVariant::Array data;
			reg.type = TypeConverter::conv(unit.registerType());
			reg.address = unit.startAddress();
			reg.length = unit.valueCount();
			data = unit.values();
			emit dataUpdated(reg, data);
		});
        QObject::connect(this, &ModbusMasterProvider::reconnect, [this](){
            mbDev->disconnectDevice();
            mbDev->connectDevice();
        });
        m_constate = QModbusDevice::UnconnectedState;
	} else {
		lFatal("ModbusMasterProvider::initBase must be called after mbDev && config allocation");
	}
}


void ModbusMasterProvider::onRestartFillConfigCmn(Config *cfg)
{
    if (!cfg->addr) {
        cfg->addr = 1;
    }
    if (!cfg->timeout) {
        cfg->timeout = 5000;
    }
    if (!cfg->attempts) {
        cfg->attempts = 3;
    }
    if (!cfg->pollTo) {
        cfg->pollTo = 100;
    }
    if (!cfg->cycleTo) {
        cfg->cycleTo = 1000;
    }
}


ModbusMasterProvider::ModbusMasterProvider(QObject *parent)
        : ProtocolClientProvider(parent)
{
	mbDev = nullptr;
	config = nullptr;
}


ModbusMasterProvider::~ModbusMasterProvider()
{
	qDebug() << "~ModbusMasterProvider";
    if (mbDev) {
        devDisconnect();
	}
	delete config;
}
