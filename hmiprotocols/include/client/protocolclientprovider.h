#ifndef PROTOCOLCLIENTPROVIDER_H
#define PROTOCOLCLIENTPROVIDER_H

#include <QTimer>
#include "protocolprovider.h"

class ProtocolClientProvider : public ProtocolProvider
{
    Q_OBJECT
public:
    ProtocolClientProvider(QObject *parent = nullptr) : ProtocolProvider(parent) {
        reconnectTimer = new QTimer(this);
        QObject::connect(reconnectTimer, &QTimer::timeout, this, &ProtocolClientProvider::reconnect);
        m_reconnectTimeOut = 1000;
        m_holdconnect = false;
    }
    virtual ~ProtocolClientProvider() = default;
    inline void setReconnectTimeout(int to) {
        m_reconnectTimeOut = to;
    }

protected slots:
    inline void onConnection() {
        reconnectTimer->stop();
    }
    inline void onDisconnection() {
        if (m_holdconnect)
            reconnectTimer->start(m_reconnectTimeOut);
    }

signals:
    void reconnect();

protected:
    int m_constate;
    int m_reconnectTimeOut;

    inline void clConnect() {
        m_holdconnect = true;
        reconnectTimer->start(m_reconnectTimeOut);
    }
    inline void clDisconnect() {
        m_holdconnect = false;
        reconnectTimer->stop();
    }

private:
    bool m_holdconnect;
    QTimer *reconnectTimer;
};

#endif // PROTOCOLCLIENTPROVIDER_H
