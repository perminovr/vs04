TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT +=
LIBS +=

HEADERS += \
	iwifistation.h

SOURCES +=


include(../root.pri)


TARGET = $${PRJ_LIB}/iwifistation


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

