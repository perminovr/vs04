TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
QT += network
LIBS += -lnl-3 -lnl-cli-3 -lnl-genl-3 -lnl-idiag-3 -lnl-nf-3 -lnl-route-3

HEADERS += \
	networkingprovider.h \
	networkingcommon.h \
	systemnetworking.h \
	networking.h \
	nlwrapper.h \
	netlinkworker.h

SOURCES += \
	networking.cpp \
	nlwrapper.cpp


include(../root.pri)
include(../platform/platform.pri)
include(../idhcpclient/idhcpclient.pri)


TARGET = $${PRJ_LIB}/networking


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
