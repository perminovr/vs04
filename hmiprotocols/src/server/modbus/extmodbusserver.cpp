#include "extmodbusserver.h"


bool ExtModbusServer::writeData(const QModbusDataUnit &newData, bool notify)
{
	if (!self)
		return false;

	if (m_map.find(newData.registerType()) == m_map.end())
		return false;

	bool hit = false;
	bool changeRequired = false;
	auto allReg = m_map.equal_range(newData.registerType());

	for (auto it = allReg.first; it != allReg.second; ++it) {
		QModbusDataUnit &current = it->second;
		if (!current.isValid()) {
			continue;
		}

		// check range start is within internal map range
		int internalRangeEndAddress = current.startAddress() + current.valueCount() - 1;
		if (newData.startAddress() < current.startAddress() || newData.startAddress() > internalRangeEndAddress) {
			continue;
		}

		// check range end is within internal map range
		int rangeEndAddress = newData.startAddress() + newData.valueCount() - 1;
		if (rangeEndAddress < current.startAddress() || rangeEndAddress > internalRangeEndAddress) {
			continue;
		}

		hit = true;

		for (uint i = 0; i < newData.valueCount(); i++) {
			const quint16 newValue = newData.value(i);
			const int translatedIndex = newData.startAddress() - current.startAddress() + i;
			changeRequired |= (current.value(translatedIndex) != newValue);
			current.setValue(translatedIndex, newValue);
		}
	}

	if (notify && changeRequired)
		pDataWritten(newData.registerType(), newData.startAddress(), newData.valueCount());

	return hit;
}



bool ExtModbusServer::writeData(const QModbusDataUnit &newData)
{
	return writeData(newData, true);
}


bool ExtModbusServer::readData(QModbusDataUnit *newData) const
{
	if (!self)
		return false;

	if ((!newData) || m_map.find(newData->registerType()) == m_map.end())
		return false;

	bool hit = false;
	auto allReg = m_map.equal_range(newData->registerType());

	for (auto it = allReg.first; it != allReg.second; ++it) {
		const QModbusDataUnit &current = it->second;
		if (!current.isValid()) {
			continue;
		}

		// check range start is within internal map range
		int internalRangeEndAddress = current.startAddress() + current.valueCount() - 1;
		if (newData->startAddress() < current.startAddress() || newData->startAddress() > internalRangeEndAddress) {
			continue;
		}

		// check range end is within internal map range
		const int rangeEndAddress = newData->startAddress() + newData->valueCount() - 1;
		if (rangeEndAddress < current.startAddress() || rangeEndAddress > internalRangeEndAddress) {
			continue;
		}

		hit = true;

		newData->setValues(current.values().mid(newData->startAddress() - current.startAddress(), newData->valueCount()));
	}
	return hit;
}


bool ExtModbusServer::setData(const QModbusDataUnit &newData, bool notify)
{
	return writeData(newData, notify);
}


void ExtModbusServer::insertRegister(QModbusDataUnit::RegisterType key, quint16 address, quint16 length)
{
	if (!self)
        return;

	auto allReg = m_map.equal_range(key);
	QModbusDataUnit value({key, address, length});
	for (auto it = allReg.first; it != allReg.second; ++it) {
		QModbusDataUnit &current = it->second;
		int curStart, curLen, curEnd;
		curStart = current.startAddress();
		curLen = current.valueCount();
		curEnd = curStart + curLen - 1;
		// expand storage
		if (quint16(curEnd+1) == address) {
			QVector <quint16> data = current.values();
			data.resize(curLen + length);
			current.setValues(data);
			current.setValueCount(curLen + length);
			return;
		}
	}
	// pushback
	m_map.emplace(key, value);
}


void ExtModbusServer::clearRegisterMap()
{
	m_map.clear();
}


ExtModbusServer::ExtModbusServer()
{
	provider = nullptr;
	self = nullptr;
}
