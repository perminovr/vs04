#ifndef UISERVICEMENU_H
#define UISERVICEMENU_H

#include <QObject>
#include <QtQml>

/*!
 * @class UIUserCtl
 * @brief Qml-синглтон, предназначенный для связывания UI пользовательских функций с внутренней реализацией
*/
class UIUserCtl : public QObject
{
	Q_OBJECT

public:
	explicit UIUserCtl(QObject *parent = nullptr) : QObject(parent) {}
	virtual ~UIUserCtl() {
		qDebug() << "~UIUserCtl";
	}

	Q_INVOKABLE void openMenu() { emit openMenuRequested(); }

	static void qmlRegister(const char *pkgName, int mj, int mi) {
		qmlRegisterUncreatableType<UIUserCtl>(pkgName, mj, mi, "UIUserCtl",
				"Can not instantiate UIUserCtl. Please, use uiUserCtl object instead");
	}

public slots:

signals:
	// from qml to core
	void openMenuRequested();

private:

};

#endif // UISERVICEMENU_H
