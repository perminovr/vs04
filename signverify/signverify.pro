TEMPLATE = app

HEADERS += \
	pubpem.h \
	utils.h

SOURCES += \
	signverify.c \
	utils.c

include(../root.pri)

LIBS += -lssl -lcrypto


TARGET = $${PRJ_BIN}/signverify


PRJNAME = signverify

VPATH += \
	$${PRJNAME}/include \
	$${PRJNAME}/src

INCLUDEPATH += \
	$${PRJNAME}/include \
	$${PRJNAME}/src
