#ifndef UIPROTOCOL_H
#define UIPROTOCOL_H

#include <QObject>
#include <QtQml>
// #include "protocolctl.h"

#if PLC_BUILD
#	define IF_PLC_BUILD(...) __VA_ARGS__
#else
#	define IF_PLC_BUILD(...)
#endif

class UIProtocol : public QObject , public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool isComplete READ isComplete NOTIFY completed)
    Q_PROPERTY(bool isStarted READ isStarted NOTIFY started)

public:
    UIProtocol(QObject *parent = nullptr) : QObject(parent) {
        m_connected = 0;
        m_completed = false;
        m_started = false;
		// pctl = nullptr;
    }
    virtual ~UIProtocol() = default;

    inline bool isComplete() const { return m_completed; }
    inline virtual void classBegin() override {}
    inline virtual void componentComplete() override {
		// if (pctl) {
		// 	QQmlContext *context = qmlContext(this);
		// 	if (context) {
		// 		pctl->m.si.name = (context->nameForObject(this));
		// 	}
		// 	pctl->m.si.type = (this->metaObject()->className());
		// 	pctl->m.si.signature = ((size_t)this);
		// 	// todo role state
		// }
        this->m_completed = true;
        emit completed(this);
    }

    inline int connected() const { return m_connected; }
    inline bool isStarted() const { return m_started; }

public slots:
    virtual void restart() {
        m_started = true;
        emit started();
    }
    virtual void pause() = 0;
    virtual void resume() = 0;
    inline void start() { restart(); }
    virtual void onPropetyChanged() = 0;

signals:
    void connectedChanged();
    void completed(UIProtocol *);
    void started();

protected:
	// ProtocolCtl *pctl;
	// inline void initProtocolCtl(ProtocolCtlConfig *cfg) {
	// 	if (!pctl) pctl = new ProtocolCtl(this);
	// 	pctl->m.cfg = cfg;
	// }
    inline void incConnected() {
        setConnected(+1);
    }
    inline void decConnected() {
        setConnected(-1);
    }

private:
    int m_connected;
    bool m_completed;
    bool m_started;
    void setConnected(int delta) {
        int old = m_connected;
        m_connected += delta;
        if (m_connected < 0)
            m_connected = 0;
        if (old != m_connected)
            emit connectedChanged();
    }
};

#endif // UIPROTOCOL_H
