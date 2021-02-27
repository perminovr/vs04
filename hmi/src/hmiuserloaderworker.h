#ifndef HMIUSERLOADERWORKER_H
#define HMIUSERLOADERWORKER_H

#include "hmiuserloader.h"

/*!
 * @class HMIUserLoaderWorker
 * @brief Выполнение затратных операций @ref HMIUserLoader в отдельном потоке
*/
class HMIUserLoaderWorker : public QObject
{
	Q_OBJECT
public:
	HMIUserLoaderWorker() = default;
	virtual ~HMIUserLoaderWorker() {
		qDebug() << "~HMIUserLoaderWorker";
	}

public slots:
	bool removeConfig();
	void prepareUpdate();
	void prepareSysLog();
	void prepareUserLog();
	void prepareConfig();	

signals:
	void updateReady(bool result);
	void sysLogReady(bool result);
	void userLogReady(bool result);
	void configReady(bool result);
};

#endif // HMIUSERLOADERWORKER_H
