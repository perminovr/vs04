#ifndef HMIGUIPROVIDER_H
#define HMIGUIPROVIDER_H

#include <QObject>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include "uipanelctl.h"
#include "uiuserctl.h"

/*
unused
*/
class HMIGuiProvider : public QObject
{
	Q_OBJECT
public:
	HMIGuiProvider(QObject *parent = nullptr) : QObject(parent) {
		userCfg = nullptr;
		serviceMenu = nullptr;
		activeEngine = nullptr;
	}
	~HMIGuiProvider() {}

public slots:
	void newServiceMenu(UIPanelCtl *uiPanelCtl, const QUrl &path) {
		serviceMenu = new QQmlApplicationEngine(this);
		serviceMenu->rootContext()->setContextProperty("uiPanelCtl", uiPanelCtl);
		serviceMenu->load(path);
		emit newServiceMenuCreated();
	}
	void newUserCfg(UIUserCtl *uiUserCtl, const QUrl &path) {
		userCfg = new QQmlApplicationEngine(this);
		connect(userCfg, &QQmlApplicationEngine::objectCreated, this, &HMIGuiProvider::onUserCfgCreated);
		userCfg->rootContext()->setContextProperty("uiUserCtl", uiUserCtl);
		userCfg->load(path);
		// next step is onUserCfgCreated
	}
	void destroyUserConfig() {
		activeEngine = serviceMenu;
		setVisible(serviceMenu, true);
		// cleanup
		if (userCfg) {
			userCfg->deleteLater();
			userCfg = nullptr;
		}
		emit userCfgDestroyed();
	}
	void toggleEngineVisible() {
		if (userCfg) {
			if (activeEngine == userCfg) {
				setVisible(userCfg, false);
				activeEngine = serviceMenu;
			} else {
				setVisible(serviceMenu, false);
				activeEngine = userCfg;
			}
			setVisible(activeEngine, true);
			emit engineToggled(activeEngine == serviceMenu);
		}
	}

signals:
	void newServiceMenuCreated();
	void userCfgReady(bool success);
	void userCfgDestroyed();
	void userCfgClosed();
	void engineToggled(bool servMenu);

private slots:
	void onUserCfgCreated(QObject *object, const QUrl &url) {
		Q_UNUSED(url);
		activeEngine = serviceMenu;
		if (object) {
			QQuickWindow *window = getWindow(userCfg);
			if (window) {
				//connect(window, &QQuickWindow::visibilityChanged, this, &HMIWorker::onUserCfgClosed); todo
				activeEngine = userCfg;
				setVisible(serviceMenu, false);
			}
		}
		setVisible(activeEngine, true);
		emit userCfgReady(object != nullptr);
	}
	void onUserCfgClosed() { // todo
		QQuickWindow *window = getWindow(userCfg);
		if (window && window->visibility() == QQuickWindow::Hidden) {
			activeEngine = serviceMenu;
			setVisible(activeEngine, true);
			emit userCfgClosed();
		}
	}

private:
	QQmlApplicationEngine *activeEngine;
	QQmlApplicationEngine *userCfg;
	QQmlApplicationEngine *serviceMenu;

	QQuickWindow *getWindow(QQmlApplicationEngine *engine) {
		if (engine) {
			QObject *obj = engine->rootObjects().first();
			if ( strcmp(obj->metaObject()->className(), "QQuickApplicationWindow") == 0 )
				return static_cast<QQuickWindow *>(obj);
		}
		return nullptr;
	}
	void setVisible(QQmlApplicationEngine *engine, bool visible) {
		if (engine) {
			//if (visible) {
				QQuickWindow *window = getWindow(engine);
				if (window) {
					window->setVisible(visible);
					//window->raise();
				}
			//}
		}
	}

};

#endif // HMIGUIPROVIDER_H
