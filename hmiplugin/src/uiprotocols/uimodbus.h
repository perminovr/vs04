#ifndef UIMODBUS_H
#define UIMODBUS_H

#include "uimbregister.h"
#include "uiprotocol.h"

class UIModbus : public UIProtocol
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int address READ address WRITE setAddress NOTIFY addressChanged)

public:
	UIModbus(QObject *parent = nullptr);
	virtual ~UIModbus();

	Q_INVOKABLE UIMbRegister *addReg(int type, QObject *item,
			QString property, int address, int length, bool byEvent = true);
	Q_INVOKABLE UIMbRegister *addReg(UIMbRegister *reg);

	struct Key {
		UIMbRegister::Type type;
		int address;
		int length;
	};

    int address();

public slots:
	// from qml to core
	void changeValue(UIMbRegister *reg, const QVariant &val);
    virtual void restart() override;

	void setAddress(int );

    virtual void onPropetyChanged() override;

signals:
    void addressChanged();

protected:
    int m_address;
	QMap <Key, UIMbRegister *> m_items;

	virtual void updateDatabase(UIMbRegister *reg, const QVariant &val) = 0;
	virtual void insertRegister(UIMbRegister *reg) = 0;

private slots:
	void addOnComplete(UITag *tag);
};

bool operator==(const UIModbus::Key &k1, const UIModbus::Key &k2);
bool operator<(const UIModbus::Key &k1, const UIModbus::Key &k2);

#endif // UIMODBUS_H
