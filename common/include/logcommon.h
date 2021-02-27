#ifndef LOGCOMMON_H
#define LOGCOMMON_H

#include <QDebug>
#include <QString>
#include <QLoggingCategory>

class LogHandler;

/*!
 * @class LogHandlerPtr
 * @brief Синглтон класс-контейнер для @ref LogHandler
*/
class LogHandlerPtr {
public:
	static inline void *self() { return m_self; }
private:
	friend class LogHandler;
	static void *m_self; 		//!< указатель на экземпляр @ref LogHandler
};

/*!
 * @class LoggingBase
 * @brief Базовый класс для объектов, использующих логгирование
*/
class LoggingBase
{
public:
	LoggingBase() { loggingCategory = nullptr; }
	~LoggingBase() = default;
	QLoggingCategory *loggingCategory;	//!< категория логов объекта
};

/*!
 * @fn registerCategory
 * @brief Регистрация категории объекта
*/
extern QLoggingCategory *registerCategory(const char *name);

/*
Для работы с логгированием использовать макросы
*/

#define INIT_LOGGER(name) this->loggingCategory = registerCategory(name)
#define GET_LOGGER(object) object->loggingCategory

#define lFatal if (LogHandlerPtr::self()) qFatal
#define lNote(object) if (LogHandlerPtr::self() && object->loggingCategory) qCInfo((*object->loggingCategory))
#define lWarn(object) if (LogHandlerPtr::self() && object->loggingCategory) qCWarning((*object->loggingCategory))
#define lError(object) if (LogHandlerPtr::self() && object->loggingCategory) qCCritical((*object->loggingCategory))
#define lDebug(object) if (LogHandlerPtr::self() && object->loggingCategory) qCDebug((*object->loggingCategory))

#define logNote() lNote(this)
#define logWarn() lWarn(this)
#define logError() lError(this)
#define logDebug() lDebug(this)

#endif // LOGCOMMON_H
