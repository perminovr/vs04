#include "loghandler.h"
#include <QFile>
#include <iostream>
#include "logcommon.h"
#include <QElapsedTimer>
#include <QFileInfo>


QFile *rotateLog(QFile *file, const QString &fname, int maxLogSize)
{
	if (file && file->size() > maxLogSize) {
		QString oldLog = fname + "0";
		QFile of(oldLog);
		of.remove();
		of.close();
		file->close();
		file->rename(oldLog);
		delete file;
		file = new QFile(fname);
		file->open(QFile::WriteOnly | QFile::Append);
	}
	return file;
}


void LogHandler::rotate()
{
	sysLogFile = rotateLog(sysLogFile, sysLogFileName, maxLogSize);
	userLogFile = rotateLog(userLogFile, userLogFileName, maxLogSize);
}


void LogHandler::LogOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	if (msg.size() < 1024) {
		char buf[2048] = {0};
		if (self) {
			/*
			 * 1) user console.*("")
				context.category == qml
			*/
			// user debugging
			if (strstr("qml", context.category) != 0) {
				quint64 ts = self->timer->elapsed();
				QFileInfo fi (context.file);
				if (self->userLogFile) {
					// FULL: '[ms] (FILE:LINE): MSG\n'
					sprintf(buf, "[%s] (%s:%d) %s\n",
							QString::number(ts).rightJustified(11, ' ').toLocal8Bit().constData(),
							fi.fileName().toLocal8Bit().data(),
							context.line,
							msg.toLocal8Bit().data());
					self->userLogFile->write(buf, strlen(buf));
					self->rotate();
				}
				emit self->newQmlMsg(fi.fileName(), context.line, ts, msg);
			}
			// system log
			else {
				bool fatal = false;
				const char *msgType = nullptr;
				LogHandler::Flags logType = Flags::Disable;
				switch (type) {
				case QtDebugMsg:
					logType = Flags::Debug;
					if (self->flags & logType) msgType = "DBG";
					break;
				case QtInfoMsg:
					logType = Flags::Note;
					if (self->flags & logType) msgType = "NOTE";
					break;
				case QtWarningMsg:
					logType = Flags::Warn;
					if (self->flags & logType) msgType = "WARN";
					break;
				case QtCriticalMsg:
					logType = Flags::Error;
					if (self->flags & logType) msgType = "ERR";
					break;
				case QtFatalMsg:
					logType = Flags::Error;
					fatal = true;
					msgType = "FATAL";
					break;
				}
				// FULL: '[ms] TYPE CAT (FILE:LINE): MSG\n'
				if (msgType) {
					// get working time
					quint64 ts = self->timer->elapsed();
					// print to buf
					// ... 'CAT (FILE:LINE): MSG\n'
					char info[512] = {0};
					char *pi = info;
					if (context.category) {
						sprintf(pi, "%s", context.category); pi += strlen(pi);
					}
					if (fatal) {
						sprintf(pi, " (%s:%d)", context.file, context.line); pi += strlen(pi);
					}
					sprintf(pi, ": %s", msg.toStdString().c_str()); pi += strlen(pi);
					// FULL
					sprintf(buf, "[%s] %s %s\n", QString::number(ts).rightJustified(11, ' ').toLocal8Bit().constData(), msgType, info);

					if (self->sysLogFile) {
						self->sysLogFile->write(buf, strlen(buf));
						self->rotate();
					}
					std::cout << buf;
					std::cout.flush();
					emit self->newLogMsg(static_cast<int>(logType), ts, info);
				}
				if (fatal) // todo && (strstr(context.category, "default") || strstr(context.category, "core")))
					abort();
			}
		} else {
			sprintf(buf, "[%s] (%s:%d)\n", msg.toLocal8Bit().data(), context.file, context.line);
			std::cout << buf;
			std::cout.flush();
		}
	}
}


QLoggingCategory *LogHandler::registerCategory(const char *name)
{
	if (name) {
		for (auto &x : this->categories) {
			if (strcmp(x->categoryName(), name) == 0)
				return x;
		}
		QLoggingCategory *ret = new QLoggingCategory(name, QtMsgType::QtWarningMsg);
		this->categories.push_back(ret);
		return ret;
	}
	return nullptr;
}


void LogHandler::setCategoryFlags(QLoggingCategory *category, Flags flags)
{
	if (category) {
		category->setEnabled(QtMsgType::QtDebugMsg, (flags & Flags::Debug)? true : false);
		category->setEnabled(QtMsgType::QtInfoMsg, (flags & Flags::Note)? true : false);
		category->setEnabled(QtMsgType::QtWarningMsg, (flags & Flags::Warn)? true : false);
		category->setEnabled(QtMsgType::QtCriticalMsg, (flags & Flags::Error)? true : false);
	}
}


void LogHandler::setCategoryFlags(const char *name, Flags flags)
{
	if (name) {
		for (auto &x : this->categories) {
			if (strcmp(x->categoryName(), name) == 0) {
				setCategoryFlags(x, flags);
				return;
			}
		}
	}
}


void LogHandler::setFlags(Flags flags)
{
	this->flags = flags;
}


LogHandler *LogHandler::self = nullptr;
void *LogHandlerPtr::m_self = nullptr;


LogHandler *LogHandler::instance(QObject *parent, const QString &sysLogFile, const QString &userLogFile, int maxLogSize)
{
	if (!self && parent && sysLogFile.length() && userLogFile.length() && maxLogSize) {
		self = new LogHandler(sysLogFile, userLogFile, maxLogSize, parent);
		LogHandlerPtr::m_self = (void*)self;
	}
	return self;
}


LogHandler::LogHandler(const QString &sysLogFile, const QString &userLogFile, int maxLogSize, QObject *parent) : QObject(parent)
{
	this->timer = new QElapsedTimer();
	this->flags = Flags::Disable;
	this->maxLogSize = maxLogSize;
	if (maxLogSize) {
		this->sysLogFileName = sysLogFile;
		this->userLogFileName = userLogFile;
		if (sysLogFile.length()) {
			this->sysLogFile = new QFile(sysLogFile);
			this->sysLogFile->open(QFile::WriteOnly | QFile::Append);
		} else {
			this->sysLogFile = nullptr;
		}
		if (userLogFile.length()) {
			this->userLogFile = new QFile(userLogFile);
			this->userLogFile->open(QFile::WriteOnly | QFile::Append);
		} else {
			this->userLogFile = nullptr;
		}
	}
	qInstallMessageHandler(LogHandler::LogOutput);
	timer->start();
}


LogHandler::~LogHandler()
{
	qDebug() << "~LogHandler";
	LogHandlerPtr::m_self = nullptr;
	LogHandler::self = nullptr;
	delete sysLogFile;
	delete userLogFile;
	delete timer;
}


LogHandler::Flags operator|(LogHandler::Flags a, LogHandler::Flags b)
{
	return static_cast<LogHandler::Flags>(static_cast<int>(a) | static_cast<int>(b));
}


QLoggingCategory *registerCategory(const char *name)
{
	if (name && LogHandlerPtr::self()) {
		LogHandler *lh = LogHandler::instance();
		if (lh) {
			return lh->registerCategory(name);
		}
	}
	return nullptr;
}

