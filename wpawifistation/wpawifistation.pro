TEMPLATE = lib
CONFIG += dynamiclib plugin
LIBS +=

HEADERS += \
	includes.h \
	os.h \
	wpa_ctrl.h \
	common.h \
	wifictlworker.h \
	wpawifistation.h \

SOURCES += \
	wpa_ctrl.c \
	common.c \
	os_unix.c \
	wpawifistation.cpp \


include(../root.pri)
include(../iwifistation/iwifistation.pri)


TARGET = $${PRJ_LIB}/wpawifistation


CONFIG += c++11

VPATH += \
	src \
	include

INCLUDEPATH += \
	src \
	include

