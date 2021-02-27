TEMPLATE = app
QT += core
CONFIG +=
LIBS +=

HEADERS += \

SOURCES += \
	wifictl-test.cpp


include(../root.pri)
include(../iwifistation/iwifistation.pri)
include(../wpawifistation/wpawifistation.pri)


TARGET = $${PRJ_BIN}/wifictl-test


CONFIG += c++11

VPATH += \
	src \
	include

INCLUDEPATH += \
	src \
	include

