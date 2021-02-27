#ifndef HPCPSERVER_H
#define HPCPSERVER_H

#include "hpcprotocol.h"
#include <QTcpServer>


class HPCProtocolServer : public HPCProtocol
{
	Q_OBJECT

public:
	HPCProtocolServer(QObject *parent = nullptr);
	virtual ~HPCProtocolServer();

public slots:
	bool restart(const QString &serverIp, quint16 port);

signals:
	void fullSysLogRequested();
	void fullUserLogRequested();
	void configRequested();
	void cmdOpenRootReceived();
	void cmdUpdateReceived();
	void cmdRebootReceived();
	void cmdSysUpdReceived();

	/*!
	 * @fn privilegedModeOn
	 * @brief Сигнал установки привилегированного режима
	*/
	void privilegedModeChanged(bool status);
	/*!
	 * @fn privilegeError
	 * @brief Сигнал запроса привилегированной команды без получения на это прав
	*/
	void privilegeError(IdCmd cmd);

private slots:
	void handleConnection();

private:
	QTcpServer *tcpServer;
	void handleControl(IdCmd cmd, const QByteArray &prm);
	void privilegedMode(bool status);
};

#endif // HPCPSERVER_H
