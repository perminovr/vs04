#ifndef PROTOCOLINFO_H
#define PROTOCOLINFO_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QLinkedList>
#include <QVector>

#include "protocolctldataunit.h"
#include "protocolctlconfig.h"

#undef Q_PROPERTY_IMPLEMENTATION_M
#define Q_PROPERTY_IMPLEMENTATION_M(type, name, getter, setter, notifier) \
	public slots: void setter(type t) { if (this->m.name != t) { this->m.name = t; emit this->notifier(); } } \
	public: type getter() const { return this->m.name; } \
	Q_SIGNAL void notifier();

class ProtocolCtl : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int logFlags READ logFlags WRITE setLogFlags NOTIFY logFlagsChanged)
	Q_PROPERTY_IMPLEMENTATION_M(int , logFlags , logFlags , setLogFlags , logFlagsChanged)

public:

	enum ProtocolRole {
		Client,
		Server
	};
	enum ProtocolState {
		Running,
		Paused
	};

	struct ShortInfo {
		size_t signature;
		QString type;
		QString name;
		ProtocolCtl::ProtocolRole role;
		ProtocolCtl::ProtocolState state;
		QByteArray toBytes() const;
		int fromBytes(const quint8 *array);
	};

	struct ServerInfo {
		QString slotAddress;
		int clients;
		QVector <QString> clientsAddr;
		QByteArray toBytes() const;
		int fromBytes(const quint8 *array);
	};

	struct ClientInfo {
		QString slotAddress;
		QString serverAddress;
		bool connectStatus;
		QByteArray toBytes() const;
		int fromBytes(const quint8 *array);
	};

	struct FullInfo {
		ShortInfo si;
		union {
			ServerInfo servInfo;
			ClientInfo clientInfo;
		};
		//
		int logFlags;
		QVector <ProtocolCtlDataUnit> db;
		ProtocolCtlConfig *cfg;
		//
		FullInfo(){ cfg = nullptr; }
		~FullInfo(){ delete cfg; }
		QByteArray toBytes() const;
		int fromBytes(const quint8 *array);
	} m;

    ProtocolCtl(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ProtocolCtl() = default;

public slots:
    inline void cmdRestart() { emit restartRequested(); }
    inline void cmdPause() { emit pauseRequested(); }
    inline void cmdResume() { emit resumeRequested(); }

signals:
    void restartRequested();
    void pauseRequested();
    void resumeRequested();
};

// todo uiprotocols init macros

#endif // PROTOCOLINFO_H
