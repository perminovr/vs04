TEMPLATE = lib
CONFIG += qt plugin
QT += qml quick core quickwidgets
QT -= gui

greaterThan(QT_MINOR_VERSION, 11) {
	QT += virtualkeyboard
} else {
	QTPLUGIN += qtvirtualkeyboardplugin
}

HEADERS += \

SOURCES += \

include(../root.pri)


TARGET = $${PRJ_LIB}/qmlqhmiplugin


VPATH += \
	include \
	src

INCLUDEPATH += \
	include \
	src

DISTFILES += \
	imports/Hmi/plugins.qmltypes \
	imports/Hmi/qmldir \
	utils/qmlplugindump.sh

install_utils.commands = \
	cp -rf $${PWD}/utils/* $${OUT_PWD}/

install_imports.commands = \
	cp -rf $${PWD}/imports ../

QMAKE_EXTRA_TARGETS += install_utils install_imports
POST_TARGETDEPS += install_utils install_imports

QMAKE_POST_LINK += \
	cp -f $(DESTDIR)/$(TARGET) ../imports/Hmi ; \
	sh qmlplugindump.sh $${QMAKE_QMAKE} Hmi 1.0 ../imports $${PRJ_LIB} ; \
	cp -rf ../imports/Hmi/plugins.qmltypes $${PWD}/imports/Hmi
