#ifndef PROTOCOLPROVIDER_H
#define PROTOCOLPROVIDER_H

#include "logcommon.h"

class ProtocolProvider : public QObject , public LoggingBase
{
	Q_OBJECT
public:
    ProtocolProvider(QObject *parent = nullptr) : QObject(parent) {
        m_usingSysComSlot = false;
    }
    virtual ~ProtocolProvider() {}

    struct Config {
        // protocol-specific config
    };

public slots:
    virtual void restart(const Config *config) = 0;

	virtual bool pause() = 0;
	virtual bool resume() = 0;

protected:
    bool m_usingSysComSlot;
};

#endif // PROTOCOLPROVIDER_H
