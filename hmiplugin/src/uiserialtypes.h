#ifndef UISERIALTYPES_H
#define UISERIALTYPES_H

#include <QObject>

class UISerialTypes {
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

#endif // UISERIALTYPES_H
