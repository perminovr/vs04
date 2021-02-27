#ifndef MBREGISTER_H
#define MBREGISTER_H

#include <QtGlobal>

struct MbRegister {
	enum Type {
		NotDef,
		InputRegister,
		HoldingRegister,
        DiscreteInputs,
		Coils
	} type;
	quint16 address;
	quint16 length;
};

#endif
