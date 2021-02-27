#ifndef PIPECLIENT_H
#define PIPECLIENT_H

#include <QLocalSocket>
#include <QObject>

class PipeClient : public QObject
{
	Q_OBJECT

public:
	PipeClient(const QString &serverName, QObject *parent = nullptr);
	virtual ~PipeClient();

	bool start();
	bool write(const QByteArray &data);

signals:
	void connected();
	void writeComplete();

private slots:
	void onBytesWritten(quint64);

private:
	QString serverName;
	QLocalSocket *socket;
};

#endif // PIPECLIENT_H
