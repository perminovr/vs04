#ifndef IFTPSERVER_H
#define IFTPSERVER_H

#include <QObject>
#include <QString>
#include <QtGlobal>

/*!
 * @class IFtpServer
 * @brief
*/
class IFtpServer : public QObject
{
	Q_OBJECT
public:
	virtual bool runDaemon(const QString &workingDir, const QString &login, const QString &password, quint16 port) = 0;
	virtual bool stopDaemon() = 0;

	IFtpServer(QObject *parent = nullptr) : QObject(parent) {}
	virtual ~IFtpServer() {}

public slots:
	virtual void restart() = 0;

protected:
	bool running;
	QString workingDir;
	QString login;
	QString password;
	quint16 port;
};

#endif // IFTPSERVER_H
