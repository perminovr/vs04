#include "uitag.h"
#include <QMetaProperty>
#include <QMetaMethod>
#include <QVariant>


#define scast(t,v) static_cast<t>(v)
#define toC() toStdString().c_str()
#define toStr() toString().toC()
#define DirectUniqueConnection \
	scast(Qt::ConnectionType, (scast(int, Qt::UniqueConnection) | scast(int, Qt::DirectConnection)))


#define CONLIST_PROP_NAME "connected_list"


UITagPtrList* UITag::getConnectedList(QObject *item, bool createIfNull)
{
	if (item) {
		QVariant vlist = item->property(CONLIST_PROP_NAME);
		return vlist.isValid()?
				vlist.value<UITagPtrList*>() : (createIfNull)?
				new UITagPtrList() : nullptr;
	}
	return nullptr;
}


bool UITag::connectToPropertyChanged(QObject *receiver, const char *slot)
{
	if (receiver && slot && m_item && m_property.length()) {
		auto clist = getConnectedList(m_item);
		if (m_list) { // tag has any list...
			// but item has not -> fail (multiple items are not allowed: only one tag for item)
			// or
			// and the item has own list, but they are different -> fail
			if (!clist || (m_list != clist)) {
				return false;
			}
		}
		const QMetaObject* meta = m_item->metaObject();
		QMetaProperty prop = meta->property(meta->indexOfProperty(m_property.toC()));
		if (prop.hasNotifySignal()) {
			QMetaMethod signal = prop.notifySignal();
			QMetaMethod updateSlot = receiver->metaObject()->method(
					receiver->metaObject()->indexOfSlot(slot));
			if (connect(m_item, signal, receiver, updateSlot, DirectUniqueConnection)) {
				if (!m_list) { // only if tag has not item list
					m_list = getConnectedList(m_item, true);
					if (m_list) {
						QVariant vlist;
						m_list->push_back(this);
						vlist.setValue(m_list);
						return m_item->setProperty(CONLIST_PROP_NAME, vlist);
					}
				} else {
					// only connect to slot (one item property (one tag) for multiple slots case)
					return true;
				}
			}
		}
	}
	return false;
}


void UITag::classBegin()
{}

void UITag::componentComplete()
{
	this->m_complete = true;
	emit completed(this);
}

bool UITag::isComplete()
{
	return this->m_complete;
}


QObject *UITag::item()
{
	return this->m_item;
}

QString UITag::property()
{
	return this->m_property;
}

bool UITag::byEvent()
{
	return this->m_byEvent;
}


void UITag::setItem(QObject *item)
{
	this->m_item = item;
	emit this->itemChanged(item);
}

void UITag::setProperty(const QString &property)
{
	this->m_property = property;
	emit this->propertyChanged(this->m_property);
}

void UITag::setByEvent(bool byEvent)
{
	this->m_byEvent = byEvent;
	emit this->byEventChanged(this->m_byEvent);
}


UITag::UITag(QObject *item, QString property, bool byEvent, QObject *parent) : QObject(parent)
{
	m_item = item;
	m_property = property;
	m_byEvent = byEvent;
	m_complete = false;
	m_list = getConnectedList(item);
}


UITag::UITag(UITag *tag, QObject *parent) : QObject(parent)
{
	if (tag) {
		m_item = tag->m_item;
		m_property = tag->m_property;
		m_byEvent = tag->m_byEvent;
		m_complete = tag->m_complete;
		m_list = tag->m_list;
	} else {
		m_item = nullptr;
		m_property = "";
		m_byEvent = true;
		m_complete = false;
		m_list = nullptr;
	}
}


UITag::UITag(QObject *parent) : QObject(parent)
{
	m_item = nullptr;
	m_property = "";
	m_byEvent = true;
	m_complete = false;
	m_list = nullptr;
}


UITag::~UITag()
{
	if (m_list) {
		m_list->removeOne(this);
		if (m_list->size() == 0) {
			delete m_list;
		}
	}
}
