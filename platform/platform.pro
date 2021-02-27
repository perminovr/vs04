TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt

HEADERS += \
	linuxgpio.h \
	linuxgpioout.h \
	linuxled.h \
	platform.h

SOURCES += \
	platform.cpp


include(../root.pri)


TARGET = $${PRJ_LIB}/platform


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
