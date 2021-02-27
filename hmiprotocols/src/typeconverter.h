#ifndef TYPECONVERTER_H
#define TYPECONVERTER_H

#include "mbregister.h"
#include "serialtypes.h"
#include <QModbusDataUnit>
#include <QSerialPort>

class TypeConverter {
public:
	static MbRegister::Type conv(const QModbusDataUnit::RegisterType &type) {
		switch (type) {
			case QModbusDataUnit::InputRegisters: return MbRegister::InputRegister;
			case QModbusDataUnit::HoldingRegisters: return MbRegister::HoldingRegister;
            case QModbusDataUnit::DiscreteInputs: return MbRegister::DiscreteInputs;
            case QModbusDataUnit::Coils: return MbRegister::Coils;
			default: return MbRegister::NotDef;
		}
	}
	static QModbusDataUnit::RegisterType conv(const MbRegister::Type &type) {
		switch (type) {
			case MbRegister::InputRegister: return QModbusDataUnit::InputRegisters;
			case MbRegister::HoldingRegister: return QModbusDataUnit::HoldingRegisters;
            case MbRegister::DiscreteInputs: return QModbusDataUnit::DiscreteInputs;
            case MbRegister::Coils: return QModbusDataUnit::Coils;
			default: return QModbusDataUnit::Invalid;
		}
	}
	static QSerialPort::StopBits conv(const SerialTypes::StopBits &stopBits) {
		switch (stopBits) {
			case SerialTypes::StopBits::OneAndHalfStop: return QSerialPort::StopBits::OneAndHalfStop;
			case SerialTypes::StopBits::TwoStop: return QSerialPort::StopBits::TwoStop;
			default: return QSerialPort::StopBits::OneStop;
		}
	}
	static QSerialPort::Parity conv(const SerialTypes::Parity &parity) {
		switch (parity) {
			case SerialTypes::Parity::Even: return QSerialPort::Parity::EvenParity;
			case SerialTypes::Parity::Odd: return QSerialPort::Parity::OddParity;
			default: return QSerialPort::Parity::NoParity;
		}
	}
};

#endif // TYPECONVERTER_H
