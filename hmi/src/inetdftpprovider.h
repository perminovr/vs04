#ifndef INETDFTPPROVIDER_H
#define INETDFTPPROVIDER_H

#include "iftpserver.h"

class QProcess;

/*!
 * @class InetdFtpProvider
 * @brief Реализация FTP сервера. Запуск через inetd
*/
class InetdFtpProvider : public IFtpServer
{
    Q_OBJECT
public:
	virtual bool runDaemon(const QString &workingDir, const QString &login, const QString &password, quint16 port) override;
	virtual bool stopDaemon() override;

    InetdFtpProvider(const QString &defaultConfPath, QObject *parent = nullptr);
    virtual ~InetdFtpProvider();

public slots:
	virtual void restart() override;

private:
	QProcess *proc;
	QString confPath;
};

#endif // INETDFTPPROVIDER_H
