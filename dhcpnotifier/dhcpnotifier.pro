TEMPLATE = app
QT += network core

HEADERS += \
	dhcpnotifier.h

SOURCES += \
	main.cpp


include(../root.pri)
include(../idhcpclient/idhcpclient.pri)


TARGET = $${PRJ_BIN}/dhcpnotifier


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
