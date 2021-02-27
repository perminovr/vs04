
PRJ_LIB = ../usr/lib
PRJ_BIN = ../usr/bin

LIBS += \
	-L$${PRJ_LIB}

include(common/common.pri)

DEFINES += "PLC_BUILD=1"
DEFINES += "PLC_EMU_BUILD=0"


QMAKE_CFLAGS += -O0
QMAKE_CXXFLAGS += -O0


target.path = /var/run/vs04-prj
INSTALLS += target

QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
