TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += network
LIBS +=

HEADERS += \
	protocolctl.h \
	protocolctlconfig.h \
	protocolctldataunit.h \
	protocolctlhandler.h \

SOURCES += \
	protocolctl.cpp \
	protocolctlhandler.cpp \


include(../root.pri)


TARGET = $${PRJ_LIB}/protocolctl


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

