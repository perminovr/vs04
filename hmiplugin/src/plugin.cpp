#include "hmiplugin.h"
#include "plugin.h"

#include "uipanelctl.h"
#include "uiuserctl.h"
#include "uinetaddress.h"
#include "uinetroute.h"
#include "uiserialport.h"
#include "modbus/uimodbustcpslave.h"
#include "modbus/uimodbusrtuslave.h"
#include "modbus/uimodbustcpmaster.h"
#include "modbus/uimodbusrtumaster.h"


#define QMLREG_PRM  QML_PACKAGE_NAME,QML_PACKAGE_MAJOR,QML_PACKAGE_MINOR


void QHmiPlugin::qmlRegister()
{
	UIPanelCtl::qmlRegister(QMLREG_PRM);
	UIUserCtl::qmlRegister(QMLREG_PRM);
	UIMbRegister::qmlRegister(QMLREG_PRM);
	UINetAddress::qmlRegister(QMLREG_PRM);
	UINetRoute::qmlRegister(QMLREG_PRM);
	UISerialPort::qmlRegister(QMLREG_PRM);
	UIModbusTcpSlave::qmlRegister(QMLREG_PRM);
	UIModbusRtuSlave::qmlRegister(QMLREG_PRM);
	UIModbusTcpMaster::qmlRegister(QMLREG_PRM);
	UIModbusRtuMaster::qmlRegister(QMLREG_PRM);
}


void QHmiPlugin::registerTypes(const char *uri)
{
	Q_ASSERT(uri == QLatin1String(QML_PACKAGE_NAME));
	QHmiPlugin::qmlRegister();
}
