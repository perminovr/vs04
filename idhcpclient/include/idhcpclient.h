#ifndef IDHCPCLIENT_H
#define IDHCPCLIENT_H

#include <QObject>
#include <QNetworkAddressEntry>

class PipeClient;
class PipeServer;

/*!
 * @class IDhcpClient
 * @brief Абстрактный класс, реализующий функции отправки/получения dhcp-событий.
 * Поведение: внешний dhcp-демон вызывает исполняемый файл dhcp-уведомителя. Уведомитель
 * через функции класса IDhcpClient сообщает о полученных настройках в основную программу.
*/
class IDhcpClient : public QObject
{
	Q_OBJECT
public:
	enum Role {
		Notifier,
		Implementer
	};
	enum DhcpState {
		Running,
		Stopped
	};

	virtual void start(Role role = Role::Notifier);
	void notify(const QString &iface, const QString &ip, const QString &mask, const QString &defGateway, bool add);

	virtual bool runDhcpDaemon(const QString &iface) = 0;
	virtual bool stopDhcpDaemon(const QString &iface) = 0;

	IDhcpClient(QObject *parent = nullptr);
	virtual ~IDhcpClient();

signals:
	void isReady();
	void complete();
	void notificationReceived(const QString &iface, const QNetworkAddressEntry &addr, const QHostAddress &defGateway, bool add);

	void dhcpStateChanged(DhcpState state, const QString &iface);

protected:
	Role role;

private slots:
	void onClientData(const QByteArray &data);

private:
	QString serverName;
	PipeClient *client;
	PipeServer *server;
};

#endif // IDHCPCLIENT_H
