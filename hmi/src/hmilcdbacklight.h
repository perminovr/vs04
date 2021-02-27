#ifndef HMILCDBACKLIGHT_H
#define HMILCDBACKLIGHT_H

#include <QtCore>
#include <QTimer>
#include "linuxled.h"

class HMILcdBacklight : public QObject
{
public:
	HMILcdBacklight(QObject *parent = nullptr) : QObject(parent) {
		time = 0;
		m_state = true;
		bl = new LinuxLed(QStringLiteral("LCD_Light"));
		timer = new QTimer(this);
		timer->setSingleShot(true);
		connect(timer, &QTimer::timeout, [this]{
			if (this->bl) {
				m_state = false;
				this->bl->setValue(0); // turn off
			}
		});
	}
	~HMILcdBacklight() {
		delete bl;
	}

	inline void turnOn() {
		m_state = true;
		this->bl->setValue(255);
	}
	void turnOffAfter(quint16 time) {
		quint16 t = this->time;
		this->time = time;
		if (t != time) {
			reset();
		}
	}
	inline void reset() {
		if (this->time)
			timer->start(this->time * 1000); // sec
	}
	inline void unset() {
		this->time = 0;
		timer->stop();
	}
	inline bool state() {
		return m_state;
	}

private:
	LinuxLed *bl;
	quint16 time;
	QTimer *timer;
	bool m_state;
};

#endif // HMILCDBACKLIGHT_H
