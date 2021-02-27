#include "uimodbusmaster.h"
#include "uitypeconverter.h"
#include <QDebug>

#define scast(t,v) static_cast<t>(v)
#define UIRegToMbReg(reg) \
	{UITypeConverter::conv(reg->type()), scast(quint16, reg->address()), scast(quint16, reg->length())}
#define DirectUniqueConnection \
    scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


void UIModbusMaster::restart()
{
    #if PLC_BUILD
        UIProtocol::restart();
        this->provider->pause();
        if (m_startFromWrite) {
            this->provider->forceWriteAllDatabase();
        }
	#endif
}


void UIModbusMaster::pause()
{
	IF_PLC_BUILD(this->provider->pause());
}


void UIModbusMaster::resume()
{
	IF_PLC_BUILD(this->provider->resume());
}


void UIModbusMaster::updateDatabase(UIMbRegister *reg, const QVariant &val)
{
	#if PLC_BUILD
		if (provider) {
            if (reg->isWritable()) {
                provider->updateDatabase(UIRegToMbReg(reg), val);
			}
		}
	#endif
}


void UIModbusMaster::insertRegister(UIMbRegister *reg)
{
	#if PLC_BUILD
		if (provider) {
            connect(reg, &UIMbRegister::isReadableChanged, this, &UIModbusMaster::onRegIsRdWrChanged, DirectUniqueConnection);
            connect(reg, &UIMbRegister::isWritableChanged, this, &UIModbusMaster::onRegIsRdWrChanged, DirectUniqueConnection);
            provider->setPolledRegister(UIRegToMbReg(reg), reg->isReadable(), reg->isWritable());
		}
	#endif
}


void UIModbusMaster::onRegIsRdWrChanged()
{
	#if PLC_BUILD
		UIMbRegister *reg = qobject_cast<UIMbRegister*>(sender());
        if (reg) {
            provider->setPolledRegister(UIRegToMbReg(reg), reg->isReadable(), reg->isWritable());
		}
	#endif
}


void UIModbusMaster::initBase()
{
	#if PLC_BUILD
		if (provider) {
			// this->pctl = provider->ctl(); // todo
			connect(provider, &ModbusMasterProvider::dataUpdated, [this](const MbRegister &reg, const HMIVariant::Array &allData){
				int endAddr = reg.address + reg.length - 1;
				int curAddr = reg.address;
				auto dataRemainder = allData;
				while (curAddr <= endAddr) {
					auto mi = this->m_items.find( {UITypeConverter::conv(reg.type), scast(quint16, curAddr), 1} );
					if (mi != this->m_items.end()) {
						bool ok;
						UIMbRegister *regItem = mi.value();
						HMIVariant::Array data = dataRemainder.mid(0, regItem->length());
						if (data.size() > 0) {
							dataRemainder = dataRemainder.mid(regItem->length());
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
						curAddr += regItem->length();
					} else {
						break;
					}
				}
			});
            connect(provider, &ModbusMasterProvider::connectedChange, [this](bool state){
				if (state) {
					lDebug(this->provider) << "connected";
                    this->incConnected();
				} else {
                    lDebug(this->provider) << "disconnected";
                    this->decConnected();
				}
				emit connectedChanged();
            });
		} else {
			qFatal("UIModbusMaster::initBase must be called after provider allocation");
		}
	#endif
}


UIModbusMaster::UIModbusMaster(QObject *parent) : UIModbus(parent)
{
	IF_PLC_BUILD(provider = nullptr);
	m_timeOut = 0;
	m_attempts = 0;
	m_pollTo = 0;
	m_cycleTo = 0;
    m_startFromWrite = false;
}
