#ifndef UIMODBUSMASTER_H
#define UIMODBUSMASTER_H

#include "uimbregister.h"
#include "uimodbus.h"
#if PLC_BUILD
#   include "modbus/modbusmasterprovider.h"
#endif

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(type t) { if (this->m_##name != t) { this->m_##name = t; emit this->notifier(); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
    protected: type m_##name;

class UIModbusMaster : public UIModbus
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

	Q_PROPERTY(int timeOut READ timeOut WRITE setTimeOut NOTIFY timeOutChanged)
	Q_PROPERTY(int attempts READ attempts WRITE setAttempts NOTIFY attemptsChanged)
	Q_PROPERTY(int pollTo READ pollTo WRITE setPollTo NOTIFY pollToChanged)
	Q_PROPERTY(int cycleTo READ cycleTo WRITE setCycleTo NOTIFY cycleToChanged)
    Q_PROPERTY(bool startFromWrite READ startFromWrite WRITE setStartFromWrite NOTIFY startFromWriteChanged)
	Q_PROPERTY_IMPLEMENTATION(int , timeOut , timeOut , setTimeOut , timeOutChanged)
	Q_PROPERTY_IMPLEMENTATION(int , attempts , attempts , setAttempts , attemptsChanged)
	Q_PROPERTY_IMPLEMENTATION(int , pollTo , pollTo , setPollTo , pollToChanged)
	Q_PROPERTY_IMPLEMENTATION(int , cycleTo , cycleTo , setCycleTo , cycleToChanged)
    Q_PROPERTY_IMPLEMENTATION(bool , startFromWrite , startFromWrite , setStartFromWrite , startFromWriteChanged)

public:
	UIModbusMaster(QObject *parent = nullptr);
	virtual ~UIModbusMaster() = default;

public slots:
	virtual void restart() override;
	virtual void pause() override;
	virtual void resume() override;

    void onPropetyChanged() override { UIModbus::onPropetyChanged(); }

protected:
	IF_PLC_BUILD(ModbusMasterProvider *provider);
	virtual void updateDatabase(UIMbRegister *reg, const QVariant &val) override;
	virtual void insertRegister(UIMbRegister *reg) override;

	void initBase();

private slots:
    void onRegIsRdWrChanged();
};

#endif // UIMODBUSMASTER_H
