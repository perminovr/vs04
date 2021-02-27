#include "platform.h"
#include <QSysInfo>
#include <QString>

namespace platform {

bool isArm()
{
	static int res = -1;
	if (res < 0) {
		QString arch = QSysInfo::currentCpuArchitecture();
		res = (arch.compare("arm") == 0)? 1 : 0;
	}
	return (res > 0)? true : false;
}

}
