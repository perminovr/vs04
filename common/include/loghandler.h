#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QList>
#include "logcommon.h"

class QFile;
class QElapsedTimer;

/*!
 * @class LogHandler
 * @brief Обработчик логгирования. Осуществляет вывод лога через @ref LogHandler::LogOutput .
*/
class LogHandler : public QObject
{
	Q_OBJECT
public:
	enum Flags {
		Disable = 0,
		Note = 0x01,
		Warn = 0x02,
		Error = 0x04,
		Debug = 0x20,
	};

	static LogHandler *instance(QObject *parent = nullptr,
			const QString &sysLogFile = "",
			const QString &userLogFile = "",
			int maxLogSize = 0);
	static void LogOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

	QLoggingCategory *registerCategory(const char *name);
	void setCategoryFlags(const char *name, Flags flags);
	void setCategoryFlags(QLoggingCategory *category, Flags flags);

public slots:
	void setFlags(Flags flags);

signals:
	void newLogMsg(int type, quint64 timestamp, const QString &msg);
	void newQmlMsg(const QString &fileName, int line, quint64 timestamp, const QString &msg);

private:
	LogHandler(const QString &sysLogFile, const QString &userLogFile, int maxLogSize = 0, QObject *parent = nullptr);
	virtual ~LogHandler();
	Q_DISABLE_COPY(LogHandler)

	static LogHandler *self;

	int maxLogSize;
	QString sysLogFileName;
	QString userLogFileName;
	QFile *sysLogFile;
	QFile *userLogFile;
	Flags flags;
	QList <QLoggingCategory *> categories;

	QElapsedTimer *timer;

	void rotate();
};

LogHandler::Flags operator|(LogHandler::Flags a, LogHandler::Flags b);
#define LOGFLAGS_ALL static_cast <LogHandler::Flags> (0xFF)
#define LOGFLAGS_BASE (LogHandler::Note | LogHandler::Warn | LogHandler::Error)

#endif // LOGHANDLER_H
