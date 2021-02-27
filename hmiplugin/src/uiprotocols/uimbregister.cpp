#include "uimbregister.h"


UIMbRegister::UIMbRegister(Type type, QObject *item, QString property,
				 int address, int length, bool byEvent, QObject *parent)
	: UITag(item, property, byEvent, parent)
{
	this->m_type = type;
	this->m_address = address;
	this->m_length = length;
	this->m_isReadable = true;
	switch (type) {
	case UIMbRegister::HR:
	case UIMbRegister::Co:
		this->m_isWritable = true;
		break;
	default:
		this->m_isWritable = false;
		break;
	}
}


void UIMbRegister::initDefault()
{
	this->m_type = UIMbRegister::IR;
	this->m_address = 0;
	this->m_length = 0;
	this->m_isReadable = true;
	this->m_isWritable = true;
}


UIMbRegister::UIMbRegister(UIMbRegister *item, QObject *parent)
	: UITag(qobject_cast<UITag*>(item), parent)
{
	if (item) {
		this->m_type = item->type();
		this->m_address = item->address();
		this->m_length = item->length();
		this->m_isReadable = item->isReadable();
		this->m_isWritable = item->isWritable();
	} else {
		initDefault();
	}
}


UIMbRegister::UIMbRegister(QObject *parent)
	: UITag(parent)
{
	initDefault();
}
