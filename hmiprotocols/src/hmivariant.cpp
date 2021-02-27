#include "hmivariant.h"


bool HMIVariant::QVariantToArray(const QVariant &var, HMIVariant::Array &array, quint16 size)
{
	const QVariant::DataPtr &ptr = var.data_ptr();
	const char *raw = &ptr.data.c;

	switch (var.type()) {
		case QVariant::String: {
			QString *data = (QString *)raw;
			quint16 tSz = (quint16)data->size()+1;
			array.resize(size);
			quint16 *p = (quint16 *)data->toStdString().c_str();
			size <<= 1;
			bzero(array.data(), size);
			memcpy(array.data(), p, qMin(size, tSz));
			return true;
		} break;
		case QVariant::ByteArray: {
			QByteArray *data = (QByteArray *)raw;
			quint16 tSz = (quint16)data->size();
			array.resize(size);
			quint16 *p = (quint16 *)data->data();
			size <<= 1;
			bzero(array.data(), size);
			memcpy(array.data(), p, qMin(size, tSz));
			return true;
		} break;

		case QVariant::Bool:
		case QVariant::Int:
		case QVariant::UInt:
		case QVariant::LongLong:
		case QVariant::ULongLong:
		case QVariant::Char: {
			quint16 tSz = sizeof(ptr.data);
			array.resize(size);
			quint16 *p = (quint16 *)raw;
			size <<= 1;
			bzero(array.data(), size);
			memcpy(array.data(), p, qMin(size, tSz));
			return true;
		} break;

		case QVariant::Double: {
			quint16 tSz = sizeof(quint16)*2;
			array.resize(size);
			float f = var.toFloat();
			quint16 *p = (quint16 *)&f;
			size <<= 1;
			bzero(array.data(), size);
			memcpy(array.data(), p, qMin(size, tSz));
			return true;
		} break;

		default: {
			return false;
		} break;
	}
}


bool HMIVariant::ArrayToQVariant(const HMIVariant::Array &array, QVariant &var)
{
	QVariant::DataPtr &ptr = var.data_ptr();
	char *raw = &ptr.data.c;

	switch (var.type()) {
		case QVariant::String: {
			HMIVariant::Array copy = array;
			copy.push_back(0); // null term
			const char *data = (const char *)copy.data();
			QString *dest = (QString *)raw;
			*dest = QString::fromStdString(data);
			return true;
		} break;
		case QVariant::ByteArray: {
			QByteArray *dest = (QByteArray *)raw;
			dest->resize(array.size()*2);
			memcpy(dest->data(), array.data(), dest->size());
			return true;
		} break;

		case QVariant::Bool:
		case QVariant::Int:
		case QVariant::UInt:
		case QVariant::LongLong:
		case QVariant::ULongLong:
		case QVariant::Char: {
			const quint16 tSz = sizeof(ptr.data);
			memcpy(raw, array.data(), qMin((quint16)array.size(), tSz));
			return true;
		} break;

		case QVariant::Double: {
			// const quint16 tSz = sizeof(quint16)*2;
			var.setValue(*((float*)array.data()));
			return true;
		} break;

		default: {
			return false;
		} break;
	}
}
