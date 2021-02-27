#ifndef LINUXGPIOOUT_H
#define LINUXGPIOOUT_H

#include "linuxgpio.h"


class LinuxGpioOut : public LinuxGpio {
public:
	LinuxGpioOut(int num, bool inverse = false) : LinuxGpio(num, LinuxGpio::Direction::GpioOutput, inverse) {
		this->prevVal = false;
	}
	~LinuxGpioOut() = default;

	/*!
	* @fn GpioOut::setOnTime
	* @brief: установить сигнал
	*/
	bool setVal(bool val) {
		this->prevVal = val;
		if (this->inverse)
			val = !val;
		QFile f(this->value);
		if (f.open(QFile::WriteOnly)) {
			auto res = f.write((val)? "1" : "0", 1);
			return (res == 1)? true : false;
		}
		return false;
	}

protected:
	bool prevVal;	//!< сохранение значения до изменения
};

#endif // LINUXGPIOOUT_H
