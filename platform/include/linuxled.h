#ifndef LINUXLED_H
#define LINUXLED_H

#include <QString>
#include <QFile>

class LinuxLed {
public:
	LinuxLed(const QString &ledName) {
		this->file = QStringLiteral("/sys/class/leds/") + ledName + QStringLiteral("/brightness");
	}
	~LinuxLed() = default;

	void setValue(qint8 val) {
		QFile f(file);
		if (f.open(QFile::WriteOnly)) {
			QString data = QString::number(val);
			f.write(data.toLocal8Bit().data(), data.size());
		}
	}

private:
	QString file;
};

#endif // LINUXLED_H
