#ifndef UITAG_H
#define UITAG_H

#include <QObject>
#include <QString>
#include <QQmlParserStatus>
#include <QLinkedList>

class UITag;

typedef QLinkedList<UITag*> UITagPtrList;
Q_DECLARE_METATYPE(UITagPtrList*);

class UITag : public QObject , public QQmlParserStatus
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

	Q_PROPERTY(QObject* item READ item WRITE setItem NOTIFY itemChanged)
	Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)
	Q_PROPERTY(bool byEvent READ byEvent WRITE setByEvent NOTIFY byEventChanged)
	Q_PROPERTY(bool isComplete READ isComplete NOTIFY completed)

public:
	UITag(QObject *item, QString property, bool byEvent = true, QObject *parent = nullptr);
	UITag(UITag *tag, QObject *parent = nullptr);
	UITag(QObject *parent = nullptr);
	virtual ~UITag();

	QObject *item();
	QString property();
	bool byEvent();

	virtual void classBegin() override;
	virtual void componentComplete() override;
	bool isComplete();

	bool connectToPropertyChanged(QObject *receiver, const char *slot);
	static UITagPtrList* getConnectedList(QObject *item, bool createIfNull = false);

public slots:
	void setItem(QObject *);
	void setProperty(const QString &);
	void setByEvent(bool);

signals:
	void itemChanged(QObject *);
	void propertyChanged(QString &);
	void byEventChanged(bool);

	void completed(UITag *);

protected slots:

protected:
	QObject *m_item;
	QString m_property;
	bool m_byEvent;
	bool m_complete;
	UITagPtrList *m_list;
};

#endif // UITAG_H
