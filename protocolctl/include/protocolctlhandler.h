#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <QTcpSocket>

#include "protocolctl.h"

class ProtocolCtlHandler : public QObject
{
	Q_OBJECT
public:

	/*!
	 * @fn add
	 * @brief
	*/
	void add(ProtocolCtl *pc);

	/*!
	 * @fn del
	 * @brief
	*/
	void del(ProtocolCtl *pc);

	bool isConnected() const;

	/*!
	 * @fn restart
	 * @brief Перезапуск протокола по установившемуся соединению
	*/
	bool restart(QTcpSocket *socket);
	/*!
	 * @fn close
	 * @brief Останов протокола; закрытие соединения
	*/
	void close();

	ProtocolCtlHandler(QObject *parent = nullptr);
	virtual ~ProtocolCtlHandler();

signals:
	/*!
	 * @fn connected
	 * @brief Сигнал установки соединения
	*/
	void connected();
	/*!
	 * @fn disconnected
	 * @brief Сигнал разрыва соединения
	*/
	void disconnected();

private slots:
	/*!
	 * @fn read
	 * @brief Слот для получения данных с соединения
	*/
	void read();

private:
	QTcpSocket *socket;
	QMap <size_t, ProtocolCtl *> protocols;
};

#endif // PROTOCOLHANDLER_H