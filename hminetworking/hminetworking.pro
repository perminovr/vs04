TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += network
LIBS +=

HEADERS += \
	hminetworkingprovider.h \
	hminetworkingcommon.h \
	hmisystemnetworking.h \
	udhcpprovider.h

SOURCES += \
	hminetworking.cpp \
	udhcpprovider.cpp


include(../root.pri)
include(../networking/networking.pri)
include(../platform/platform.pri)
include(../idhcpclient/idhcpclient.pri)
include(../iwifistation/iwifistation.pri)


TARGET = $${PRJ_LIB}/hminetworking


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

