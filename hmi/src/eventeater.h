#ifndef EVENTEATER_H
#define EVENTEATER_H

#include <QObject>
#include <QInputEvent>

/*!
 * @class EventEater
 * @brief Для получения событий нажатия на экран
*/
class EventEater : public QObject
{
	Q_OBJECT
public:
	EventEater(QObject *parent = nullptr) : QObject(parent) {}
	~EventEater() = default;

signals:
	void inputEventOccurred();

protected:
	bool eventFilter(QObject *obj, QEvent *event) override {
		if ( dynamic_cast<QInputEvent *>(event) ) {
			emit inputEventOccurred();
		}
		return QObject::eventFilter(obj, event);
	}
};

#endif // EVENTEATER_H
