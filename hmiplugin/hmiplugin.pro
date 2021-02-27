TEMPLATE = lib
CONFIG += qt dynamiclib plugin
QT += qml quick core quickwidgets
QT -= gui

greaterThan(QT_MINOR_VERSION, 11) {
	QT += virtualkeyboard
} else {
	QTPLUGIN += qtvirtualkeyboardplugin
}

HEADERS += \
	plugin.h \
	hmiplugin.h \
	uiprotocol.h \
	uiserialtypes.h \
	uitag.h \
	uimbregister.h \
	uipanelctl.h \
	uiuserctl.h \
	uinetaddress.h \
	uinetroute.h \
	uiserialport.h \
	uimodbus.h \
	uitypeconverter.h \
	\
	modbus/uimodbusslave.h \
	modbus/uimodbustcpslave.h \
	modbus/uimodbusrtuslave.h \
	\
	modbus/uimodbusmaster.h \
	modbus/uimodbustcpmaster.h \
	modbus/uimodbusrtumaster.h \

SOURCES += \
	plugin.cpp \
	uitag.cpp \
	uimbregister.cpp \
	uinetaddress.cpp \
	uinetroute.cpp \
	uiserialport.cpp \
	uimodbus.cpp \
	\
	modbus/uimodbusslave.cpp \
	modbus/uimodbustcpslave.cpp \
	modbus/uimodbusrtuslave.cpp \
	\
	modbus/uimodbusmaster.cpp \
	modbus/uimodbustcpmaster.cpp \
	modbus/uimodbusrtumaster.cpp \

include(../root.pri)
include(../hmiprotocols/hmiprotocols.pri)
include(../hminetworking/hminetworking.pri)


TARGET = $${PRJ_LIB}/qmlqhmiplugin


VPATH += \
	include \
	src \
	src/uicomslots \
	src/uiprotocols	\
	src/uiprotocols/client \
	src/uiprotocols/server \

INCLUDEPATH += \
	include \
	src \
	src/uicomslots	\
	src/uiprotocols	\
	src/uiprotocols/server	\
	src/uiprotocols/client	\

DISTFILES += \
	imports/Hmi/plugins.qmltypes \
	imports/Hmi/qmldir \
	utils/qmlplugindump.sh

install_utils.commands = \
	cp -rf $${PWD}/utils/* $${OUT_PWD}/

install_imports.commands = \
	cp -rf $${PWD}/imports ../

QMAKE_EXTRA_TARGETS += install_utils install_imports
POST_TARGETDEPS += install_utils install_imports

QMAKE_POST_LINK += \
	cp -f $(DESTDIR)/$(TARGET) ../imports/Hmi ; \
	sh qmlplugindump.sh $${QMAKE_QMAKE} Hmi 1.0 ../imports $${PRJ_LIB} ; \
	cp -rf ../imports/Hmi/plugins.qmltypes $${PWD}/imports/Hmi
