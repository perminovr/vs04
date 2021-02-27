#include "inetdftpprovider.h"

#include <QObject>
#include <QString>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QtGlobal>

#define INETD_CTL QStringLiteral("/usr/bin/inetdctl.sh")
#define CONF_PREFIX QStringLiteral("_ftpprov")


static inline bool delConfig(const QString &conf)
{
	QFile file(conf);
	file.remove();
	return true;
}


static inline bool doConfig(const QString &conf, const QString &workingDir = "", const QString &login = "", const QString &password = "", quint16 port = 0)
{
	// PORT stream tcp nowait root /usr/sbin/ftpd ftpd -w WORKDIR -n NAME -p PASS
	QFile file(conf);
	if (file.open(QFile::WriteOnly | QFile::Truncate)) {
		QString pattern = QString("%1 stream tcp nowait root /usr/sbin/ftpd ftpd -w %2 -n %3 -p %4")
				.arg(port).arg(workingDir).arg(login).arg(password);
		QTextStream ts(&file);
		ts << "\n" << pattern << "\n";
		return true;
	}
	return false;
}


bool InetdFtpProvider::runDaemon(const QString &workingDir, const QString &login, const QString &password, quint16 port)
{
	if (this->proc && !this->running) {
		if (workingDir.length() && login.length() && password.length() && port) {
			this->workingDir = workingDir;
			this->login = login;
			this->password = password;
			this->port = port;
			bool ok = doConfig(this->confPath, workingDir, login, password, port);
			if (ok)
				ok = this->proc->startDetached();
			if (ok)
				this->running = true;
			return ok;
		}
	}
	return false;
}


bool InetdFtpProvider::stopDaemon()
{
	if (this->proc && this->running) {
		bool ok = delConfig(this->confPath);
		if (ok)
			ok = this->proc->startDetached();
		if (ok)
			this->running = false;
		return ok;
	}
	return false;
}


void InetdFtpProvider::restart()
{
	stopDaemon();
	runDaemon(workingDir, login, password, port);
}


InetdFtpProvider::InetdFtpProvider(const QString &defaultConfPath, QObject *parent) : IFtpServer(parent)
{
	this->confPath = defaultConfPath + CONF_PREFIX;
	this->running = false;
	if (QFile(INETD_CTL).exists()) {
		this->proc = new QProcess(this);
		proc->setStandardOutputFile(QProcess::nullDevice());
		proc->setStandardErrorFile(QProcess::nullDevice());
		proc->setProgram(INETD_CTL);
	} else {
		this->proc = nullptr;
		qCritical() << "Couldn't find" << INETD_CTL;
	}
}


InetdFtpProvider::~InetdFtpProvider()
{
	qDebug() << "~InetdFtpProvider";
    stopDaemon();
}

