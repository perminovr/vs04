#ifndef PROTOCOLCTLDATAUNIT_H
#define PROTOCOLCTLDATAUNIT_H

#include <QObject>
#include <QVariant>
#include <QVector>

class ProtocolCtlDataUnit : public QObject
{
	Q_OBJECT
public:
	size_t signature;
	QString reg;
	QVariant::Type type;

    ProtocolCtlDataUnit() : QObject(nullptr) { value = nullptr; size = 0; };
	ProtocolCtlDataUnit(const ProtocolCtlDataUnit &);
    virtual ~ProtocolCtlDataUnit() { delete value; }

	void setValue(const quint8 *v, quint32 sz) {
		if (v && sz) {
			if (this->size < sz) {
				delete this->value;
				this->value = new quint8[sz];
			}
			this->size = sz;
			memcpy(this->value, v, this->size);
		}
	}
	const quint8 *getValue() const { return value; }
	quint32 getSize() const { return size; }

	QByteArray toBytes() const;
	int fromBytes(const quint8 *array);

signals:
	void changed();

private:
	quint32 size;
	quint8 *value;
};

#endif // PROTOCOLCTLDATAUNIT_H
