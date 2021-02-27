TEMPLATE = app

QT += quick core quickwidgets

greaterThan(QT_MINOR_VERSION, 11) {
	QT += virtualkeyboard
} else {
	QTPLUGIN += qtvirtualkeyboardplugin
}

CONFIG += lang-ru_RU lang-en_GB

HEADERS += \
	config.h \
	hmiworker.h \
	hmidatabase.h \
	hmiuserloader.h \
	hmiuserloaderworker.h \
	hmilcdbacklight.h \
	eventeater.h \
	hmiguiprovider.h \
	hmiqmlanalyzer.h \
	iftpserver.h \
	inetdftpprovider.h \


SOURCES += \
	main.cpp \
	hmiworker.cpp \
	hmidatabase.cpp \
	hmiuserloader.cpp \
	inetdftpprovider.cpp \

RESOURCES += \
	src/qml.qrc


include(../root.pri)
include(../platform/platform.pri)
include(../wdtctl/wdtctl.pri)
include(../idhcpclient/idhcpclient.pri)
include(../networking/networking.pri)
include(../hminetworking/hminetworking.pri)
include(../hmiprotocols/hmiprotocols.pri)
include(../hmiplugin/hmiplugin.pri)
include(../ssdp/ssdp.pri)
include(../hpcprotocol/hpcprotocol.pri)
include(../qzip/qzip.pri)
include(../iwifistation/iwifistation.pri)
include(../wpawifistation/wpawifistation.pri)


TARGET = $${PRJ_BIN}/hmi


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

QMAKE_CXXFLAGS += -Wno-missing-field-initializers

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH += /media/roman/data/linux-sys/opt/build/sysroots/vs04/mtax_am335x/usr/lib/qml
QML_IMPORT_PATH += /opt/build/sysroots/vs04/mtax_am335x/usr/lib/qml
