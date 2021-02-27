TEMPLATE = app

HEADERS += \
	src/COMPort.h \
	src/Observer.h \
	src/SimplestCOM.h

SOURCES += \
	src/COMPort.cpp \
	src/SimplestCOM.cpp \
	src/main.cpp

include(../root.pri)

LIBS += \

TARGET = $${PRJ_BIN}/simplestCOM


PRJNAME = simplestCOM

VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
