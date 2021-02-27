TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += 
LIBS += 

HEADERS += \
	logcommon.h \
	loghandler.h
	

SOURCES += \
	loghandler.cpp
	

include(../root.pri)
LIBS -= -lcommon


TARGET = $${PRJ_LIB}/common


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

