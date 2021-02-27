#include "protocolctl.h"

#define scast(t,v) static_cast<t>(v)
#define baseFromArray(var, arrmemb, tpsz) (decltype(var)) ( (tpsz) (*((const char*)(&(arrmemb)))) )
#define getBaseFromArray(var, arrmemb, tpsz) var=baseFromArray(var, arrmemb, tpsz)
#define getBaseFromArrayAndShift(var, arr, it, tpsz) { getBaseFromArray(var, arr[it], tpsz); it+=sizeof(tpsz); }
#define strFromArray(arr) ((const char *)arr)
#define getStringFromArrayAndShift(str, arr, it) { const char *tstr = strFromArray(arr); it += strlen(tstr) + 1; str = tstr; }


QByteArray ProtocolCtl::ShortInfo::toBytes() const
{
	QByteArray ret;
	ret.reserve(128);
	ret.append((const char *)(&this->signature), sizeof(size_t)); // 4 or 8
	if (sizeof(size_t) == 4) {
		ret.append(4, (char)0); // up to 8
	}
	ret.append(this->type);
	ret.push_back((char)0);
	ret.append(this->name);
	ret.push_back((char)0);
	ret.append((const char *)(&this->role), sizeof(quint8));
	ret.append((const char *)(&this->state), sizeof(quint8));
	return ret;
}


int ProtocolCtl::ShortInfo::fromBytes(const quint8 *array)
{
	int it = 0;
	getBaseFromArrayAndShift(this->signature, array, it, size_t);
	if (sizeof(size_t) == 4) {
		it += 4; // up to 8
	}
	getStringFromArrayAndShift(this->type, array, it);
	getStringFromArrayAndShift(this->name, array, it);
	getBaseFromArrayAndShift(this->role, array, it, quint8);
	getBaseFromArrayAndShift(this->state, array, it, quint8);
	return it;
}


QByteArray ProtocolCtl::ServerInfo::toBytes() const
{
	QByteArray ret;
	ret.reserve(128);
	ret.append(this->slotAddress);
	ret.push_back((char)0);
	ret.append((const char *)(&this->clients), sizeof(quint32));
	for (const auto &x : this->clientsAddr) {
		ret.append(x);
		ret.push_back((char)0);
	}
	return ret;
}


int ProtocolCtl::ServerInfo::fromBytes(const quint8 *array)
{
	int it = 0;
	getStringFromArrayAndShift(this->slotAddress, array, it);
	getBaseFromArrayAndShift(this->clients, array, it, quint32);
	this->clientsAddr.resize(this->clients);
	for (int i = 0; i < this->clients; ++i) {
		getStringFromArrayAndShift(this->clientsAddr[i], array, it);
	}
	return it;
}


QByteArray ProtocolCtl::ClientInfo::toBytes() const
{
	QByteArray ret;
	ret.reserve(128);
	ret.append(this->slotAddress);
	ret.push_back((char)0);
	ret.append(this->serverAddress);
	ret.push_back((char)0);
	ret.append((const char *)(&this->connectStatus), sizeof(quint8));
	return ret;

}


int ProtocolCtl::ClientInfo::fromBytes(const quint8 *array)
{
	int it = 0;
	getStringFromArrayAndShift(this->slotAddress, array, it);
	getStringFromArrayAndShift(this->serverAddress, array, it);
	getBaseFromArrayAndShift(this->connectStatus, array, it, quint8);
	return it;
}


QByteArray ProtocolCtl::FullInfo::toBytes() const
{
	QByteArray ret = this->si.toBytes();
	ret.append((const char *)(&this->logFlags), 1);
	switch (this->si.role) {
		case ProtocolRole::Server: {
			ret += this->servInfo.toBytes();
		} break;
		case ProtocolRole::Client: {
			ret += this->clientInfo.toBytes();
		} break;
	}
	int db_size = this->db.size();
	ret.append((const char *)(&db_size), sizeof(quint32));
	for (const auto &x : this->db) {
		ret += x.toBytes();
	}
	if (this->cfg) ret += this->cfg->toBytes();
	return ret;
}


int ProtocolCtl::FullInfo::fromBytes(const quint8 *array)
{
    int it = 0;
	it += this->si.fromBytes(array+it);
	getBaseFromArrayAndShift(this->logFlags, array, it, quint8);
	switch (this->si.role) {
		case ProtocolRole::Server: {
			it += this->servInfo.fromBytes(array+it);
		} break;
		case ProtocolRole::Client: {
			it += this->clientInfo.fromBytes(array+it);
		} break;
	}
	quint32 sz;
	getBaseFromArrayAndShift(sz, array, it, quint32);
	this->db.resize(sz);
	for (quint32 i = 0; i < sz; ++i) {
		it += this->db[i].fromBytes(array+it);
	}
	if (this->cfg) it += this->cfg->fromBytes(array+it);
    return it;
}


QByteArray ProtocolCtlDataUnit::toBytes() const
{
	QByteArray ret;
	ret.reserve(64);
	ret.append((const char *)(&this->signature), sizeof(size_t)); // 4 or 8
	if (sizeof(size_t) == 4) {
		ret.append(4, (char)0); // up to 8
	}
	ret.append(this->reg);
	ret.push_back((char)0);
	ret.append((const char *)(&this->type), sizeof(quint8));
	ret.append((const char *)(&this->size), sizeof(quint32));
	ret.append((const char *)(this->value), this->size);
	return ret;
}


int ProtocolCtlDataUnit::fromBytes(const quint8 *array)
{
	int it = 0;
	getBaseFromArrayAndShift(this->signature, array, it, size_t);
	if (sizeof(size_t) == 4) {
		it += 4; // up to 8
	}
	getStringFromArrayAndShift(this->reg, array, it);
	getBaseFromArrayAndShift(this->type, array, it, quint8);
	quint32 sz;
	getBaseFromArrayAndShift(sz, array, it, quint32);
	setValue(array + it, sz);
	it += this->size;
	emit this->changed();
	return it;
}


ProtocolCtlDataUnit::ProtocolCtlDataUnit(const ProtocolCtlDataUnit &du) : QObject(du.parent())
{
	reg = du.reg;
	type = du.type;
	setValue(du.value, du.size);
}
