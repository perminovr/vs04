#ifndef LINUXGPIO_H
#define LINUXGPIO_H

#include <QString>
#include <QFile>

class LinuxGpio {
public:
	enum Direction {
		GpioInput,
		GpioOutput,
	};

	/*!
	* @fn Gpio::getGpioNum
	* @brief: перевод порта и номера ножки в значение gpio linux
	*/
	static int getGpioNum(int port, int pin) { return port*32+pin; }

	LinuxGpio(int num, Direction dir, bool inverse = false) {
		this->inverse = inverse;
		this->num = QString::number(num);
		this->value = QStringLiteral("/sys/class/gpio/gpio") + this->num + QStringLiteral("/value");
		if ( !_isExported() )
			_export();
		_direction(dir);
	}
	~LinuxGpio() = default;

protected:
	QString num;	//!< номер gpio linux
	QString value;	//!< имя файла для чтения/записи значения
	bool inverse;	//!< инвертирование сигнала как на запись, так и на чтение

	inline bool _isExported() const {
		return QFile(value).exists();
	}
	void _export() const {
		QFile f(QStringLiteral("/sys/class/gpio/export"));
		if (f.open(QFile::WriteOnly)) {
			f.write(num.toLocal8Bit(), num.length());
		}
	}
	void _unexport() const {
		QFile f(QStringLiteral("/sys/class/gpio/unexport"));
		if (f.open(QFile::WriteOnly)) {
			f.write(num.toLocal8Bit(), num.length());
		}
	}
	void _direction(Direction dir) const {
		QString fdir = QStringLiteral("/sys/class/gpio/gpio") + this->num + QStringLiteral("/direction");
		QFile f(fdir);
		if (f.open(QFile::WriteOnly)) {
			const char *sdir = (dir == Direction::GpioInput)? "in" : "out";
			f.write(sdir, strlen(sdir));
		}
	}
};

#endif // LINUXGPIO_H
