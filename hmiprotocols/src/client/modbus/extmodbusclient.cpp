#include "extmodbusclient.h"

#define REQUESTED_LENGTH_MAX 100


static inline bool operator>(const QModbusPolledDataUnit &k1, const QModbusPolledDataUnit &k2)
{
	return k1.data.startAddress() > k2.data.startAddress();
}


static inline bool operator==(const QModbusPolledDataUnit &k1, const QModbusPolledDataUnit &k2)
{
	return k1.data.startAddress() == k2.data.startAddress() && k1.data.valueCount() == k2.data.valueCount();
}


static auto QModbusPolledDataUnitMultiMap_insert(QModbusPolledDataUnitMultiMap &map, QModbusDataUnit::RegisterType key, const QModbusPolledDataUnit &value)
{
    auto allReg = map.equal_range(key);
    for (auto it = allReg.first; it != allReg.second; ++it) {
        if (it->second == value) { // already inserted
            it->second.rdpolled = value.rdpolled;
            it->second.wrpolled = value.wrpolled;
            return it;
        }
        if (it->second > value) { // insert ascending
            return map.emplace_hint(it, key, value);
        }
    }
    return map.emplace(key, value);
}


template <class T>
static inline void QVectorSplit2p(const QVector <T> &in, QVector <T> &fpart, QVector <T> &spart, int fpartEnd, int len)
{
    fpart = (fpartEnd > 0)? in.mid(0, fpartEnd) : QVector <T> (0);
    spart = (len > 0)? in.mid(fpartEnd+len) : QVector <T> (0);
}


void ExtModbusClient::setPolledRegister(QModbusDataUnit::RegisterType key, quint16 address, quint16 length, 
		bool rdpolled, bool wrpolled)
{
	if (!self) {
		return;
	}
	QModbusPolledDataUnit value({{key, address, length}, rdpolled, wrpolled});

    QModbusPolledDataUnitMultiMap_insert(m_rdMap, key, value);
    switch (key) {
        case QModbusDataUnit::HoldingRegisters:
        case QModbusDataUnit::Coils: {
            QModbusPolledDataUnitMultiMap_insert(m_wrMap, key, value);
        } break;
        default: /* NOP */ break;
    }
}


void ExtModbusClient::setRegisterData(QModbusDataUnit::RegisterType key, quint16 address, quint16 length, 
		const QVector <quint16> &data)
{
	if (!self) {
		return;
	}

	if (data.size() == length) {
		switch (key) {
			case QModbusDataUnit::HoldingRegisters:
			case QModbusDataUnit::Coils: {
				QModbusPolledDataUnit value({{key, address, length}, false, false});
                auto allReg = m_wrMap.equal_range(key);
				for (auto it = allReg.first; it != allReg.second; ++it) {
                    if (it->second == value && it->second.wrpolled) {
                        it->second.data.setValues(data);
                        value.rdpolled = it->second.rdpolled;
                        value.wrpolled = it->second.wrpolled;
                        auto jt = QModbusPolledDataUnitMultiMap_insert(m_wrFifo, key, value);
                        jt->second.data.setValues(data);
						return;
					}
				}
			} break;
			default: /* NOP */ break;
		}
	}
}


bool ExtModbusClient::sendNextRd()
{
	// data for sending
	QModbusDataUnit data;
	data.setRegisterType(current->second.data.registerType());
	data.setStartAddress(current->second.data.startAddress());
	int count = current->second.data.valueCount(); // total reg len
	int lastEnd = data.startAddress() + count - 1; // last reg num

    // find first rd accessible
    for (auto it = current; it != m_rdMap.end(); ++it) {
        if (it->second.wrpolled) break;
        else current++;
    }
    if (current == m_rdMap.end()) {
        return false;
    }

	// gluing registers
	auto tmp = current; tmp++;
    for (auto it = tmp; it != m_rdMap.end(); ++it) {
		if (it->first == current->first) { // same reg type
			if (it->second.rdpolled) { // polling is enable for this record
				QModbusDataUnit &d = it->second.data;
				int dStart, dLen, dEnd;
				dStart = d.startAddress();
				dLen = d.valueCount();
				dEnd = dStart + dLen - 1;
				if ((lastEnd+1) >= dStart) { // may cover each other
					int sz = dEnd-lastEnd;
                    if ((count+sz) > REQUESTED_LENGTH_MAX) {
                        break;
                    }
					if (sz > 0) {
						count += sz;
						lastEnd = dEnd;
					}
					current = it;
					continue;
				}
			}
		}
		break;
	}

	current++; // switch to next record

	data.setValueCount(count);
	QModbusReply *reply = self->sendReadRequest(data, m_address);
	if (reply) {
		if (!reply->isFinished()) {
			connect(reply, &QModbusReply::finished, this, &ExtModbusClient::rdReady);
            m_busy = true;
			return true; // next after answer
		} else {
			reply->deleteLater(); // error
		}
	}
	return false;
}


bool ExtModbusClient::sendNextWrFifo()
{
    // data for sending
    QModbusDataUnit data;
    data.setRegisterType(current->second.data.registerType());
    data.setStartAddress(current->second.data.startAddress());
    int count = current->second.data.valueCount(); // total reg len
    int lastEnd = data.startAddress() + count - 1; // last reg num

    // find first wr accessible
    for (auto it = current; it != m_wrFifo.end(); ++it) {
        if (it->second.wrpolled) break;
        else current++;
    }
    if (current == m_wrFifo.end()) {
        return false;
    }

    // gluing registers
    QVector <quint16> payload = current->second.data.values();
    auto tmp = current; tmp++;
    for (auto it = tmp; it != m_wrFifo.end(); ++it) {
        if (it->first == current->first) { // same reg type
            if (it->second.wrpolled) { // polling is enable for this record
                QModbusDataUnit &d = it->second.data;
                int dStart, dLen, dEnd;
                dStart = d.startAddress();
                dLen = d.valueCount();
                dEnd = dStart + dLen - 1;
                if ((lastEnd+1) >= dStart) { // may cover each other
                    int sz = dEnd-lastEnd;
                    if ((count+sz) > REQUESTED_LENGTH_MAX) {
                        break;
                    }
                    if (sz > 0) {
                        count += sz;
                        QVector <quint16> d = it->second.data.values();
                        if (d.size() == sz) payload += d;
                        else payload += d.mid(d.size()-sz, sz);
                        lastEnd = dEnd;
                    }
                    m_wrFifo.erase(current);
                    current = it;
                    continue;
                }
            }
        }
        break;
    }

    tmp = current;
    current++; // switch to next record
    m_wrFifo.erase(tmp);

    data.setValues(payload);
    QModbusReply *reply = self->sendWriteRequest(data, m_address);
    if (reply) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, &ExtModbusClient::wrReady);
            m_busy = true;
            return true; // next after answer
        } else {
            reply->deleteLater(); // error
        }
    }
    return false;
}


void ExtModbusClient::sendNextPollingRequest()
{
    if (!self) {
        return;
    }
    if (self->state() != QModbusDevice::ConnectedState) {
        return;
    }
    if (m_busy) { // will be called later from rd/wrReady
        return;
    }
	// swich between tables
	switch (ctable) {
		case CurrentTable::RdTable:
            if (current == m_rdMap.end() || !m_wrFifo.empty()) { // nothing to read or write is required
				ctable = CurrentTable::WrTable;
                current = m_wrFifo.begin();
                pollTimer->start(0); // next immediately
            } else {
				bool ok = sendNextRd();
                if (!ok) pollTimer->start(0); // next immediately
			}
            break;
		case CurrentTable::WrTable:
            if (current == m_wrFifo.end()) {
				ctable = CurrentTable::RdTable;
                current = m_rdMap.begin();
                pollTimer->start(m_cycleTo);
			} else {
                bool ok = sendNextWrFifo();
                if (!ok) pollTimer->start(0); // next immediately
            }
            break;
	}
}


void ExtModbusClient::rdReady()
{
	QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
	if (reply) {
		auto error = reply->error();
		switch (error) {
		case QModbusDevice::NoError: {
            if (m_wrFifo.empty()) {
                emit dataIsReady(reply->result());
            } else { // do not override prepared values
                auto d = reply->result();
                auto dd = d.values();
                decltype(d) du;
                auto key = d.registerType();
                int dStart, dLen, dEnd;
                int itStart, itLen, itEnd;
                int offset = d.startAddress(); // for vector iteration
                auto allReg = m_wrFifo.equal_range(key);
                QVector<quint16> du_values, d_values;
                for (auto it = allReg.first; it != allReg.second; ++it) {
                    dStart = d.startAddress();
                    dLen = d.valueCount();
                    dEnd = dStart + dLen - 1;
                    itStart = it->second.data.startAddress();
                    itLen = it->second.data.valueCount();
                    itEnd = itStart + itLen - 1;
                    if (itStart > dEnd) { // out of range from right side
                        break;
                    }
                    if (itStart <= dEnd && itEnd >= dStart) { // cut input data
                        if (dStart == dEnd) {
                            goto reply_out; // nothing to change
                        }
                        // split data: du - emit now, d - later, between them - drop out
                        QVectorSplit2p(d.values(), du_values, d_values, itStart-dStart-offset, itLen);
                        if (du_values.size() > 0) {
                            du.setRegisterType(key);
                            du.setStartAddress(dStart);
                            du.setValues(du_values);
                            emit dataIsReady(du);
                        }
                        if (d_values.size() > 0) {
                            d.setStartAddress(itEnd+1);
                            d.setValues(d_values);
                        } else {
                            goto reply_out; // nothing to change
                        }
                    }
                }
                if (d.valueCount() > 0) {
                    emit dataIsReady(d);
                }
            }
		} break;
		default: {
			int code = (error == QModbusDevice::ProtocolError)?
					static_cast<int>(reply->rawResult().exceptionCode()) : 0;
			emit errorData(reply->errorString(), code);
		} break;
		}
reply_out:
		reply->deleteLater();
    }

    int to = m_pollTo;
    m_busy = false;
    pollTimer->start(to);
}


void ExtModbusClient::wrReady()
{
	QModbusReply *reply = qobject_cast<QModbusReply *>(sender());
	if (reply) {
		auto error = reply->error();
		switch (error) {
		case QModbusDevice::NoError: {
		} break;
		default: {
			int code = (error == QModbusDevice::ProtocolError)?
					static_cast<int>(reply->rawResult().exceptionCode()) : 0;
			emit errorData(reply->errorString(), code);
		} break;
		}
		reply->deleteLater();
	}

    int to = m_pollTo;
    m_busy = false;
    pollTimer->start(to);
}


void ExtModbusClient::setStartFromWr(bool startFromWr)
{
    m_startFromWr = startFromWr;
    if (m_startFromWr) {
        ctable = CurrentTable::RdTable;
        current = m_rdMap.end();
        m_wrFifo = m_wrMap; // all
    } else {
        ctable = CurrentTable::WrTable;
        current = m_wrFifo.end();
    }
}


void ExtModbusClient::setIsPolling(bool isPolling)
{
	m_isPolling = isPolling;
    if (m_isPolling) {
        if (!pollTimer->isActive()) {
            setStartFromWr(m_startFromWr);
            pollTimer->start(0);
		}
	} else {
		pollTimer->stop();
	}
	emit isPollingChanged();
}


void ExtModbusClient::clearRegisterMap()
{
	setIsPolling(false);
    m_wrMap.clear();
    m_rdMap.clear();
    m_wrFifo.clear();
}


void ExtModbusClient::initBase()
{
	if (self) {
		connect(self, &QModbusClient::stateChanged, this, &ExtModbusClient::stateChanged);
		connect(self, &QModbusClient::errorOccurred, this, &ExtModbusClient::errorOccurred);
	} else {
		qFatal("ExtModbusClient::initBase must be called after self allocation");
	}
}


bool ExtModbusClient::connectDevice()
{
    if (self) {
        bool ok = self->connectDevice();
        if (ok) setIsPolling(true);
        return ok;
    }
    return false;
}


void ExtModbusClient::disconnectDevice()
{
    if (self) {
        setIsPolling(false);
        self->disconnectDevice();
    }
}


ExtModbusClient::ExtModbusClient(QObject *parent) : QObject(parent)
{
	self = nullptr;
    m_busy = false;

	pollTimer = new QTimer(this);
	pollTimer->setSingleShot(true);
	connect(pollTimer, &QTimer::timeout, this, &ExtModbusClient::sendNextPollingRequest);

	m_address = 1;
	m_isPolling = false;
	m_pollTo = 0;
	m_cycleTo = 0;

    m_startFromWr = false;
	ctable = CurrentTable::WrTable;
    current = m_wrFifo.end();
}
