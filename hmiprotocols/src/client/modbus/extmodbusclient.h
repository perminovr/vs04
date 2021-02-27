#ifndef EXTMODBUSSERVER_H
#define EXTMODBUSSERVER_H

#include <map>
#include <QModbusClient>
#include <QTimer>

#undef Q_PROPERTY_IMPLEMENTATION
#define Q_PROPERTY_IMPLEMENTATION(type, name, getter, setter, notifier) \
	public slots: void setter(type t) { if (this->m_##name != t) { this->m_##name = t; emit this->notifier(); } } \
	public: type getter() const { return this->m_##name; } \
	Q_SIGNAL void notifier(); \
	protected: type m_##name;
#undef Q_PROPERTY_IMPLEMENTATION2
#define Q_PROPERTY_IMPLEMENTATION2(type, name, getter, notifier) \
    public: type getter() const { return this->m_##name; } \
    Q_SIGNAL void notifier(); \
    protected: type m_##name;

struct QModbusPolledDataUnit {
	QModbusDataUnit data;
	bool rdpolled;
	bool wrpolled;
};

typedef std::multimap <QModbusDataUnit::RegisterType, QModbusPolledDataUnit> QModbusPolledDataUnitMultiMap;

class ExtModbusClient : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int address READ address WRITE setAddress NOTIFY addressChanged)
	Q_PROPERTY(int pollTo READ pollTo WRITE setPollTo NOTIFY pollToChanged)
	Q_PROPERTY(int cycleTo READ cycleTo WRITE setCycleTo NOTIFY cycleToChanged)
	Q_PROPERTY(bool isPolling READ isPolling WRITE setIsPolling NOTIFY isPollingChanged)
    Q_PROPERTY(bool startFromWr READ startFromWr WRITE setStartFromWr NOTIFY startFromWrChanged)
	Q_PROPERTY_IMPLEMENTATION(int , address , address , setAddress , addressChanged)
	Q_PROPERTY_IMPLEMENTATION(int , pollTo , pollTo , setPollTo , pollToChanged)
	Q_PROPERTY_IMPLEMENTATION(int , cycleTo , cycleTo , setCycleTo , cycleToChanged)
    Q_PROPERTY_IMPLEMENTATION2(bool , isPolling , isPolling , isPollingChanged)
    Q_PROPERTY_IMPLEMENTATION2(bool , startFromWr , startFromWr , startFromWrChanged)

public:
	ExtModbusClient(QObject *parent = nullptr);
	virtual ~ExtModbusClient() = default;

	void clearRegisterMap();
	void setPolledRegister(QModbusDataUnit::RegisterType key, quint16 address, quint16 length,
			bool rdpolled, bool wrpolled = false);
	void setRegisterData(QModbusDataUnit::RegisterType key, quint16 address, quint16 length,
            const QVector <quint16> &data);

	inline void setTimeout(int newTimeout) { if (self) self->setTimeout(newTimeout); }
	inline void setNumberOfRetries(int number) { if (self) self->setNumberOfRetries(number); }
	inline void setConnectionParameter(int parameter, const QVariant &value) { if (self) self->setConnectionParameter(parameter, value); }
	inline QModbusDevice::State state() const { if (self) return self->state(); return QModbusDevice::UnconnectedState; }
	inline QString errorString() const { if (self) return self->errorString(); return QString(); }

public slots:
    bool connectDevice();
    void disconnectDevice();

    void setIsPolling(bool);
    void setStartFromWr(bool);

signals:
	void dataIsReady(const QModbusDataUnit &data);
	void errorData(const QString &error, int code);
	void stateChanged(int state);
	void errorOccurred(int error);

protected:
	QModbusClient *self;
    QModbusPolledDataUnitMultiMap m_rdMap;
    QModbusPolledDataUnitMultiMap m_wrMap;
    QModbusPolledDataUnitMultiMap m_wrFifo;
	QTimer *pollTimer;
	QModbusPolledDataUnitMultiMap::iterator current;
	enum CurrentTable {
		RdTable, WrTable
	} ctable;

	void initBase();

private slots:
	void sendNextPollingRequest();
	void rdReady();
	void wrReady();

private:
    bool m_busy;
    bool sendNextRd();
    bool sendNextWrFifo();
};

#endif // EXTMODBUSSERVER_H
