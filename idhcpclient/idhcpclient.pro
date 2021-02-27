TEMPLATE = lib
CONFIG += dynamiclib plugin
QT += network

HEADERS += \
	idhcpclient.h \
	pipeclient.h \
	pipeserver.h

SOURCES += \
	idhcpclient.cpp \
	pipeclient.cpp \
	pipeserver.cpp


include(../root.pri)


TARGET = $${PRJ_LIB}/idhcpclient


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
