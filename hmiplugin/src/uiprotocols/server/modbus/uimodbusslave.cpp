#include "uimodbusslave.h"
#include "uitypeconverter.h"
#include <QDebug>

#define scast(t,v) static_cast<t>(v)
#define toC() toStdString().c_str()
#define UIRegToMbReg(reg) \
	{UITypeConverter::conv(reg->type()), scast(quint16, reg->address()), scast(quint16, reg->length())}
#define DirectUniqueConnection \
    scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


void UIModbusSlave::restart()
{
	pause();
	IF_PLC_BUILD(UIModbus::restart());
}


void UIModbusSlave::pause()
{
	IF_PLC_BUILD(this->provider->pause());
}


void UIModbusSlave::resume()
{
	IF_PLC_BUILD(this->provider->resume());
}


void UIModbusSlave::updateDatabase(UIMbRegister *reg, const QVariant &val)
{
	#if PLC_BUILD
		if (provider) {
            if (reg->isReadable()) {
                provider->updateDatabase(UIRegToMbReg(reg), val);
			}
		}
	#endif
}


void UIModbusSlave::forceUpdateDb()
{
	UIMbRegister *reg = qobject_cast<UIMbRegister *>(sender());
	if (reg && reg->byEvent()) {
		QObject *item = reg->item();
		QVariant val = item->property(reg->property().toC());
		if (val.isValid()) {
			updateDatabase(reg, val);
		}
	}
}


void UIModbusSlave::insertRegister(UIMbRegister *reg)
{
	#if PLC_BUILD
		connect(reg, &UIMbRegister::isReadableChanged, this, &UIModbusSlave::forceUpdateDb, DirectUniqueConnection);
        if (provider)
            provider->insertRegister(UIRegToMbReg(reg));
	#endif
}


void UIModbusSlave::initBase()
{
	#if PLC_BUILD
		if (provider) {
			// this->pctl = provider->ctl(); // todo
			connect(provider, &ModbusSlaveProvider::dataUpdated, [this](const MbRegister &reg){
				int endAddr = reg.address + reg.length - 1;
				int curAddr = reg.address;
				while (curAddr <= endAddr) {
					Key key = {UITypeConverter::conv(reg.type), scast(quint16, curAddr), 1};
					auto mi = this->m_items.find(key);
					if (mi != this->m_items.end()) {
						bool ok;
						UIMbRegister *regItem = mi.value();
						if (regItem->isWritable()) {
                            HMIVariant::Array data = this->provider->getData(UIRegToMbReg(regItem));
							if (data.size() > 0) {
								QObject *item = regItem->item();
								if (item) {
									const char *prop = regItem->property().toStdString().c_str();
									QVariant var = item->property(prop);
									ok = HMIVariant::ArrayToQVariant(data, var);
									if (ok) {
										item->setProperty(prop, var);
									}
								}
							}
						}
						curAddr += regItem->length();
					} else {
						break;
					}
				}
			});
			// todo rework qt 14.2+
            connect(provider, &ModbusSlaveProvider::clientConnected, [this](const ModbusSlaveProvider::ClientAddress *address){
				Q_UNUSED(address)
				lDebug(this->provider) << "client connected";
				this->incConnected();
				emit connectedChanged();
            });
            connect(provider, &ModbusSlaveProvider::clientDisconnected, [this](const ModbusSlaveProvider::ClientAddress *address){
				Q_UNUSED(address)
				lDebug(this->provider) << "client disconnected";
				this->decConnected();
				emit connectedChanged();
            });
		} else {
			qFatal("UIModbusSlave::initBase must be called after provider allocation");
		}
	#endif
}


UIModbusSlave::UIModbusSlave(QObject *parent) : UIModbus(parent)
{
	IF_PLC_BUILD(provider = nullptr);
}
