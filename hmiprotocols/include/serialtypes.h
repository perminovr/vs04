#ifndef SERIALTYPES_H
#define SERIALTYPES_H

#include <QObject>

class SerialTypes {
	Q_ENUMS(Parity)
	Q_ENUMS(StopBits)
public:
	enum Parity {
		nDefParity,
		NoParity,
		Even,
		Odd
	};
	enum StopBits {
		nDefStopBits,
		OneStop,
		OneAndHalfStop,
		TwoStop
	};
};

#endif // SERIALTYPES_H