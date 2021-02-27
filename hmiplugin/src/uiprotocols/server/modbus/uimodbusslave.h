#ifndef UIMODBUSSLAVE_H
#define UIMODBUSSLAVE_H

#include "uimbregister.h"
#include "uimodbus.h"
#if PLC_BUILD
#   include "modbus/modbusslaveprovider.h"
#endif

class UIModbusSlave : public UIModbus
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

public:
	UIModbusSlave(QObject *parent = nullptr);
	virtual ~UIModbusSlave() = default;

public slots:
    virtual void restart() override;
	virtual void pause() override;
	virtual void resume() override;

	void onPropetyChanged() override { UIModbus::onPropetyChanged(); }

signals:
	void netAddrChanged();
    void portChanged();

protected:
	IF_PLC_BUILD(ModbusSlaveProvider *provider);
	virtual void updateDatabase(UIMbRegister *reg, const QVariant &val) override;
	virtual void insertRegister(UIMbRegister *reg) override;

	void initBase();

private slots:
	void forceUpdateDb();
};

#endif // UIMODBUSSLAVE_H
