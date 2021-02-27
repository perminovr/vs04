#ifndef UIMBREGISTER_H
#define UIMBREGISTER_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QtQml>

#include "uitag.h"

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(type t) { if (this->m_##name != t) { this->m_##name = t; emit this->notifier(); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
	private: type m_##name;

class UIMbRegister : public UITag
{
public:
	enum Type {
		IR = 1,
		HR,
        DI,
		Co,
	};

	Q_OBJECT

	Q_PROPERTY(UIMbRegister::Type type READ type WRITE setType NOTIFY typeChanged)
	Q_PROPERTY(int address READ address WRITE setAddress NOTIFY addressChanged)
	Q_PROPERTY(int length READ length WRITE setLength NOTIFY lengthChanged)
	Q_PROPERTY(bool isReadable READ isReadable WRITE setIsReadable NOTIFY isReadableChanged)
	Q_PROPERTY(bool isWritable READ isWritable WRITE setIsWritable NOTIFY isWritableChanged)
	Q_ENUMS(Type)

	Q_PROPERTY_IMPLEMENTATION(Type , type , type , setType , typeChanged)
	Q_PROPERTY_IMPLEMENTATION(int , address , address , setAddress , addressChanged)
	Q_PROPERTY_IMPLEMENTATION(int , length , length , setLength , lengthChanged)
	Q_PROPERTY_IMPLEMENTATION(bool , isReadable , isReadable , setIsReadable , isReadableChanged)
	Q_PROPERTY_IMPLEMENTATION(bool , isWritable , isWritable , setIsWritable , isWritableChanged)

public:
	UIMbRegister(Type type, QObject *item, QString property,
			int address, int length, bool byEvent = true, QObject *parent = nullptr);
	UIMbRegister(UIMbRegister *item, QObject *parent = nullptr);
	UIMbRegister(QObject *parent = nullptr);
	virtual ~UIMbRegister() {
		qDebug() << "~UIMbRegister";
	}

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterType<UIMbRegister>(pkgName, mj, mi, "UIMbRegister");
	}

public slots:

signals:

private:
	void initDefault();
};

#endif // UIMBREGISTER_H
