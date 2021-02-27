#ifndef EXCEPTIONWHAT_H
#define EXCEPTIONWHAT_H

#include <QString>
#include <exception>

class ExceptionWhat : public std::exception
{
public:
    ExceptionWhat(const QString &message) : m_message(message) {}
    virtual ~ExceptionWhat() { }
    virtual const char* what() const Q_DECL_NOTHROW override  { return m_message.toStdString().c_str(); }
private:
    QString m_message;
};

template <class T>
class ExceptionWhatWithResult : public ExceptionWhat
{
public:
    ExceptionWhatWithResult(const QString &message, T result) : ExceptionWhat(message), m_result(result) {}
    virtual ~ExceptionWhatWithResult() { }
	T result() const Q_DECL_NOTHROW { return m_result; }
private:
    T m_result;
};

#endif // EXCEPTIONWHAT_H