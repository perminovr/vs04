TEMPLATE = lib
CONFIG += dynamiclib plugin
CONFIG += qt
LIBS += -lz

HEADERS += \
	qzipwriter.h \
	qzipreader.h

SOURCES += \
	qzip.cpp


include(../root.pri)


TARGET = $${PRJ_LIB}/qzip


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src
