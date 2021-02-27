TEMPLATE = lib
CONFIG += dynamiclib plugin

HEADERS += \
	wdtctl.h

SOURCES += \
	wdtctl.c


include(../root.pri)


TARGET = $${PRJ_LIB}/wdtctl


PRJNAME = wdtctl

VPATH += \
	$${PRJNAME}/include \
	$${PRJNAME}/src

INCLUDEPATH += \
	$${PRJNAME}/include \
	$${PRJNAME}/src
