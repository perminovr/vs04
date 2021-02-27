TEMPLATE = lib
CONFIG += dynamiclib plugin
QT += network

HEADERS += \
	include/ssdp.h \
	ssdpserver.h

SOURCES += \
	ssdp.cpp 


include(../root.pri)


TARGET = $${PRJ_LIB}/ssdp


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
