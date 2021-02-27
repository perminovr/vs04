#ifndef PROTOCOLCTLCONFIG_H
#define PROTOCOLCTLCONFIG_H

#include <QObject>
#include <QVariant>
#include <QVector>

class ProtocolCtlConfig : public QObject
{
	Q_OBJECT
public:
	ProtocolCtlConfig(QObject *parent = nullptr) : QObject(parent) {}
	virtual ~ProtocolCtlConfig(){}

	virtual QByteArray toBytes() const = 0;
	virtual int fromBytes(const quint8 *array) = 0;

signals:
	void changed();
};

#endif // PROTOCOLCTLCONFIG_H