TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += network
LIBS +=  -lcrypto -lssl

HEADERS += \
	hpcprotocol.h \
	hpcprotocolclient.h \
	hpcprotocolserver.h

SOURCES += \
	hpcprotocol.cpp \
	hpcprotocolclient.cpp \
	hpcprotocolserver.cpp


include(../root.pri)


TARGET = $${PRJ_LIB}/hpcprotocol


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
