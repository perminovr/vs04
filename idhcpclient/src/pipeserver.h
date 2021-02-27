#ifndef PIPESERVER_H
#define PIPESERVER_H

#include <QLocalServer>
#include <QObject>

class PipeServer : public QObject
{
	Q_OBJECT

public:
	PipeServer(const QString &serverName, QObject *parent = nullptr);
	virtual ~PipeServer();

	bool start(int maxConnections);

signals:
	void dataIsReady(const QByteArray &data);

private slots:
	void handleConnection();
	void readyRead();

private:
	QString serverName;
	QLocalServer *server;
};

#endif // PIPESERVER_H
