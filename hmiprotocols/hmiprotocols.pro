TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += network serialbus serialport
LIBS +=

HEADERS += \
	hmivariant.h \
	serialportprovider.h \
	mbregister.h \
	protocolprovider.h \
	protocolclientprovider.h \
	protocolserverprovider.h \
	serialtypes.h \
	typeconverter.h \
	\
	modbus/extmodbusserver.h \
	modbus/extmodbustcpserver.h \
	modbus/extmodbusrtuserver.h \
	modbus/modbusslaveprovider.h \
	modbus/modbustcpslaveprovider.h \
	modbus/modbusrtuslaveprovider.h \
	\
	modbus/extmodbusclient.h \
	modbus/extmodbustcpclient.h \
	modbus/extmodbusrtuclient.h \
	modbus/modbusmasterprovider.h \
	modbus/modbustcpmasterprovider.h \
	modbus/modbusrtumasterprovider.h \


SOURCES += \
	hmivariant.cpp \
	serialportprovider.cpp \
	\
	modbus/extmodbusserver.cpp \
	modbus/modbusslaveprovider.cpp \
	modbus/modbustcpslaveprovider.cpp \
	modbus/modbusrtuslaveprovider.cpp \
	\
	modbus/extmodbusclient.cpp \
	modbus/modbusmasterprovider.cpp \
	modbus/modbustcpmasterprovider.cpp \
	modbus/modbusrtumasterprovider.cpp \

include(../root.pri)
include(../hminetworking/hminetworking.pri)


TARGET = $${PRJ_LIB}/hmiprotocols


VPATH += \
	include \
	include/server/ \
	include/client/ \
	src \
	src/server/ \
	src/client/ \

INCLUDEPATH += \
	include \
	include/server/ \
	include/client/ \
	src \
	src/server/ \
	src/client/ \
