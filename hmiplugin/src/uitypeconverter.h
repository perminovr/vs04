#ifndef UITYPECONVERTER_H
#define UITYPECONVERTER_H

#if PLC_BUILD
#include "mbregister.h"
#include "hminetworkingcommon.h"
#include "serialtypes.h"
#include "uimbregister.h"
#include "uinetaddress.h"
#include "uiserialtypes.h"

/*!
 * @class UITypeConverter
 * @brief Статический класс конвертации типов данных из пространства UI в пространство реализации и обратно
*/
class UITypeConverter
{
public:
	static MbRegister::Type conv(const UIMbRegister::Type &type)
	{
		switch (type) {
			case UIMbRegister::IR: return MbRegister::InputRegister;
			case UIMbRegister::HR: return MbRegister::HoldingRegister;
            case UIMbRegister::DI: return MbRegister::DiscreteInputs;
            case UIMbRegister::Co: return MbRegister::Coils;
			default: return MbRegister::NotDef;
		}
	}
	static UIMbRegister::Type conv(const MbRegister::Type &type)
	{
		switch (type) {
			case MbRegister::HoldingRegister: return UIMbRegister::HR;
			case MbRegister::InputRegister: return UIMbRegister::IR;
            case MbRegister::DiscreteInputs: return UIMbRegister::DI;
            case MbRegister::Coils: return UIMbRegister::Co;
			default: return UIMbRegister::IR;
		}
	}
	static HMINetworkingCommon::Iface conv(const UINetAddress::Interface &iface) {
		switch (iface) {
			case UINetAddress::Interface::Ethernet: return HMINetworkingCommon::Iface::EthIface;
			case UINetAddress::Interface::Wifi: return HMINetworkingCommon::Iface::WifiIface;
			default: return HMINetworkingCommon::Iface::ndefIface;
		}
	}
	static UINetAddress::Interface conv(const HMINetworkingCommon::Iface &iface) {
	   switch (iface) {
		   case HMINetworkingCommon::Iface::WifiIface: return UINetAddress::Interface::Wifi;
		   default: return UINetAddress::Interface::Ethernet;
	   }
	}
	static SerialTypes::StopBits conv(const UISerialTypes::StopBits &stopBits) {
		switch (stopBits) {
			case UISerialTypes::StopBits::OneAndHalfStop: return SerialTypes::StopBits::OneAndHalfStop;
			case UISerialTypes::StopBits::TwoStop: return SerialTypes::StopBits::TwoStop;
			default: return SerialTypes::StopBits::OneStop;
		}
	}
	static SerialTypes::Parity conv(const UISerialTypes::Parity &parity) {
		switch (parity) {
			case UISerialTypes::Parity::Even: return SerialTypes::Parity::Even;
			case UISerialTypes::Parity::Odd: return SerialTypes::Parity::Odd;
			default: return SerialTypes::Parity::NoParity;
		}
	}
};
#endif // PLC_BUILD

#endif // TYPECONVERTER_H
