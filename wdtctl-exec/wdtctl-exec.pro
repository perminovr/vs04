TEMPLATE = app

HEADERS += \
	wdtctl.h

SOURCES += \
	wdtctl.c


include(../root.pri)


TARGET = $${PRJ_BIN}/wdtctl


PRJNAME = wdtctl

VPATH += \
	../$${PRJNAME}/$${PRJNAME}/include \
	../$${PRJNAME}/$${PRJNAME}/src

INCLUDEPATH += \
	../$${PRJNAME}/$${PRJNAME}/include \
	../$${PRJNAME}/$${PRJNAME}/src
