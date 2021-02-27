#include "uimodbus.h"
#include <QDebug>

#define scast(t,v) static_cast<t>(v)
#define toC() toStdString().c_str()
#define toStr() toString().toC()


template <class T>
static inline bool interval_conj(const T &as, const T &ae, const T &bs, const T &be)
{
	return (((as) <= (be)) && ((ae) >= (bs)));
}


void UIModbus::onPropetyChanged()
{
	QObject *item = sender();
	UITagPtrList *list = UIMbRegister::getConnectedList(item);
	if (list) {
		for (auto &x : *list) {
			UIMbRegister *reg = qobject_cast<UIMbRegister *>(x);
			if (reg && reg->byEvent()) {
				QVariant val = item->property(reg->property().toC());
				if (val.isValid()) {
					updateDatabase(reg, val);
				}
			}
		}
	}
}


UIMbRegister *UIModbus::addReg(int type, QObject *item,
		QString property, int address, int length, bool byEvent)
{
	UIMbRegister *reg = nullptr;
	if (item) {
		UIMbRegister::Type regType = scast(UIMbRegister::Type, type);
		Key key = {regType, address, length};
		if (m_items.find(key) == m_items.end()) {
			reg = new UIMbRegister(regType, item, property, address, length, byEvent, this);
			if (reg) {
				reg->connectToPropertyChanged(this, "onPropetyChanged()");
				this->m_items.insert(key, reg);
				insertRegister(reg);
			}
		}
	}
	return reg;
}


void UIModbus::addOnComplete(UITag *tag)
{
	UIMbRegister *reg = qobject_cast<UIMbRegister *>(tag);
	if (reg) {
		Key key = {reg->type(), reg->address(), reg->length()};
		if (m_items.find(key) == m_items.end()) {
			reg->connectToPropertyChanged(this, "onPropetyChanged()");
			this->m_items.insert(key, reg);
			insertRegister(reg);
		}
	}
}


UIMbRegister *UIModbus::addReg(UIMbRegister *reg)
{
	if (reg) {
		if (reg->isComplete()) {
			addOnComplete(reg);
		} else {
			connect(reg, &UIMbRegister::completed, this, &UIModbus::addOnComplete);
		}
	}
	return reg;
}


void UIModbus::changeValue(UIMbRegister *reg, const QVariant &val)
{
	if (reg && reg->item()) {
		if (reg->byEvent()) {
			reg->item()->setProperty(reg->property().toC(), val);
		} else {
			updateDatabase(reg, val);
		}
	}
}


void UIModbus::restart()
{
    UIProtocol::restart();
	// update all registers
	for (auto &x : this->m_items) {
		QObject *obj = x->item();
		if (obj) {
			updateDatabase(x, obj->property(x->property().toC()));
		}
	}
}


UIModbus::UIModbus(QObject *parent) : UIProtocol(parent)
{
    m_address = 0;
}


UIModbus::~UIModbus()
{}


int UIModbus::address()
{
	return m_address;
}


void UIModbus::setAddress(int addr)
{
	m_address = addr;
	emit addressChanged();
}


bool operator==(const UIModbus::Key &k1, const UIModbus::Key &k2) {
	return (k1.type == k2.type) &&
			interval_conj(k1.address, k1.address+k1.length-1,
						  k2.address, k2.address+k2.length-1);
}

bool operator<(const UIModbus::Key &k1, const UIModbus::Key &k2) {
	return (k1.type < k2.type)? true :
			(k1 == k2)? false :
			(k1.type == k2.type && k1.address < k2.address);
}

