#include "udhcpprovider.h"


bool UdhcpProvider::runDhcpDaemon(const QString &iface)
{
	if (this->role == Role::Implementer && udhcpcs.find(iface) == udhcpcs.end()) {
		QProcess *proc = new QProcess(this);
		QStringList args;
		args << "-f"; // foreground
		args << "-p" << QString("/tmp/udhcpc.%1.pid").arg(iface); // pid of udhcpc for iface
		args << "-i" << iface; // iface
		proc->setProgram("udhcpc");
		proc->setArguments(args);
		proc->setStandardOutputFile(QProcess::nullDevice());
		proc->setStandardErrorFile(QProcess::nullDevice());
		proc->setProperty("iface", iface); // remember property
		connect(proc, &QProcess::errorOccurred, this, &UdhcpProvider::onProcErrorOccurred);
		connect(proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &UdhcpProvider::onProcFinished);
		connect(proc, &QProcess::started, this, &UdhcpProvider::onProcStarted);
		proc->start(); // isn't detached
		udhcpcs[iface] = proc;
		return true;
	}
	return false;
}


bool UdhcpProvider::stopDhcpDaemon(const QString &iface)
{
	if (this->role == Role::Implementer && udhcpcs.find(iface) != udhcpcs.end()) {
		QProcess *proc = udhcpcs[iface];
		proc->kill();
		udhcpcs.remove(iface);
		return true;
	}
	return false;
}


void UdhcpProvider::start(Role role)
{
	switch (role) {
		case Role::Implementer:
			system("killall -9 udhcpc");
			break;
		case Role::Notifier:
			break;
	}
	IDhcpClient::start(role);
}


void UdhcpProvider::onProcErrorOccurred(QProcess::ProcessError error)
{
	QProcess *proc = qobject_cast<QProcess *>(sender());
	if (proc) {
		QVariant var = proc->property("iface");
		QString iface = var.toString();
		// todo
		switch (error) {
			case QProcess::FailedToStart:
				break;
			case QProcess::Crashed:
				break;
			default:
				break;
		}
	}
}


void UdhcpProvider::onProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	Q_UNUSED(exitCode)
	Q_UNUSED(exitStatus)
	QProcess *proc = qobject_cast<QProcess *>(sender());
	if (proc) {
		QVariant var = proc->property("iface");
		// wanna to restart if crashed
		if (udhcpcs.find(var.toString()) != udhcpcs.end()) {
			proc->start();
		}
		// user finished process -> delete
		else {
			proc->deleteLater();
			emit dhcpStateChanged(DhcpState::Stopped, var.toString());
		}
	}
}


void UdhcpProvider::onProcStarted()
{
	QProcess *proc = qobject_cast<QProcess *>(sender());
	if (proc) {
		QVariant var = proc->property("iface");
		emit dhcpStateChanged(DhcpState::Running, var.toString());
	}
}


UdhcpProvider::UdhcpProvider(QObject *parent) : IDhcpClient(parent)
{}


UdhcpProvider::~UdhcpProvider()
{
	qDebug() << "~UdhcpProvider";
}

