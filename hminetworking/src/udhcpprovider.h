#ifndef UDHCPPROVIDER_H
#define UDHCPPROVIDER_H

#include "idhcpclient.h"
#include <QMap>
#include <QProcess>

class UdhcpProvider : public IDhcpClient
{
    Q_OBJECT
public:
	virtual void start(Role role = Role::Notifier) override;

	virtual bool runDhcpDaemon(const QString &iface) override;
	virtual bool stopDhcpDaemon(const QString &iface) override;

    UdhcpProvider(QObject *parent = nullptr);
    virtual ~UdhcpProvider();

private slots:
	void onProcErrorOccurred(QProcess::ProcessError error);
	void onProcFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onProcStarted();

private:
	QMap<QString, QProcess*> udhcpcs;
};

#endif // UDHCPPROVIDER_H
