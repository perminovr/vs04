#ifndef HMIVARIANT_H
#define HMIVARIANT_H

#include <QVariant>
#include <QVector>

class HMIVariant
{
public:
	HMIVariant() = default;
	~HMIVariant() = default;

	typedef QVector <quint16> Array;

	static bool QVariantToArray(const QVariant &var, Array &array, quint16 size);
	static bool ArrayToQVariant(const Array &array, QVariant &var);
};


#endif // HMIVARIANT_H
