#include "hpcprotocol.h"
#include <QHostAddress>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <sys/socket.h>


enum Reason {
	rData = 1,	//!< Данные с подтверждением
	rAck,		//!< Подтверждение
};


enum Ack {
	aOk,		//!< Кадр успешно принят
	aBusy,		//!< Кадр не может быть принят в данный момент
	aErr,		//!< Ошибка
};


#pragma pack(push,1)
struct FrameHeader {
	quint8 num;
	quint8 reason;
	quint8 to;
	quint16 dataSize;
	quint16 reserve;
	quint8 ack;
	//
	static const quint8 size = 8;
	QByteArray toBytes() const;
	void fromBytes(const QByteArray &array);
	FrameHeader(quint8 num, quint8 reason, quint8 to, quint16 dataSize, quint8 ack);
	FrameHeader(const QByteArray &array);
};
#pragma pack(pop)


static bool encryptString(EVP_CIPHER_CTX *ctx, const QString &data, QByteArray &out);
static bool decryptString(EVP_CIPHER_CTX *ctx, const char *in, QString &out);


void HPCProtocol::dataIsReady(const FrameAddress &partner, const QByteArray &payload)
{
	int size = payload.size();
	quint8 id;
	int it = 0;

	if (this->m_partner != partner) {
		this->m_partner = partner;
		emit this->partnerChanged();
	}

	// prevent multiple handling
	// lose oldest cmd
	struct {
		bool isNeedToBeDone = false;
		QByteArray prm;
	} cmdToHandle[cEnd];

	while (it < size) {
		id = payload[it];
		it++;
		switch (id) {
		case dCtl: {
			IdCmd cmd;
			// QByteArray prm;
			cmd = (IdCmd)payload[it];
			it++;
			switch (cmd) {
			case cEcho:
			case cReqAll:
			case cUpd:
			case cReqSysLog:
			case cReqUserLog:
			case cReqConfig:
			case cEnterPrivil:
			case cLeavePrivil:
			case cOpenRoot:
			case cSysReboot:
			case cSysUpdate: {
				cmdToHandle[cmd].isNeedToBeDone = true;
				it++;
			} break;
			default: {
				return;
			} break;
			}
		} break;
		case dIdent: {
			it += this->m_panel.ident.fromBytes((const quint8 *)payload.data() + it);
			emit this->panelIdentChanged();
		} break;
		case dEth: {
			it += this->m_panel.eth.fromBytes((const quint8 *)payload.data() + it);
			emit this->panelEthChanged();
		} break;
		case dState: {
			memcpy(&this->m_panel.state, payload.data() + it, 8);
			it += 8;
			emit this->panelStateChanged();
		} break;
		case dWifi: {
			it += this->m_panel.wifi.fromBytes((const quint8 *)payload.data() + it, this->ctx);
			emit this->panelWifiChanged();
		} break;
		case dShort: {
			it += this->m_panel.shortp.fromBytes((const quint8 *)payload.data() + it);
			emit this->panelShortChanged();
		} break;
		case dStrSysLog: {
			it += this->m_panel.strSysLog.fromBytes((const quint8 *)payload.data() + it);
			emit this->panelStrSysLogAdded();
		} break;
		case dStrUserLog: {
			it += this->m_panel.strUserLog.fromBytes((const quint8 *)payload.data() + it);
			emit this->panelStrUserLogAdded();
		} break;
        case dUserPassw: {
            it += this->m_panel.userPassword.fromBytes((const quint8 *)payload.data() + it, this->ctx);
            emit this->panelUserPasswordChanged();
        } break;
		default /* dEnd */: {
		} break;
		}
	}
	for (int i = 0; i < (int)cEnd; ++i) {
		if (cmdToHandle[i].isNeedToBeDone) {
			emit this->ctlReceived((IdCmd)i, cmdToHandle[i].prm);
		}
	}
}


void HPCProtocol::read()
{
	QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
	if (client && client == socket) {
		while (client->bytesAvailable()) {
			QByteArray frames = client->readAll();
			int fssize = frames.size();
			for (;fssize >= FrameHeader::size;) {
				FrameHeader header(frames);
				switch (header.reason) {
				case rData: {
					quint8 ack = m_isBusy?
							aBusy : aOk;
					FrameHeader ans ( {header.num, rAck, HPCP_TIMEOUT_DEFAULT, 0, ack} );
					client->write(ans.toBytes());
					if (ack == aOk) {
						QByteArray payload = frames.mid(FrameHeader::size, header.dataSize);
						this->dataIsReady({client->peerAddress().toString(), client->peerPort()}, payload);
					}
					fssize -= FrameHeader::size + header.dataSize;
				} break;
				case rAck: {
					switch (header.ack) {
					case aOk: {
						this->ackFailTimer->stop();
						int size = this->txframe.size();
						if (size > txSent) { // may grow up
							QByteArray payload = this->txframe.right(size - txSent);
							this->txframe.clear();
							this->prepareDataToSend(payload);
						} else { // done -> clear
							this->txframe.clear();
						}
					} break;
					default: {
						repeatTimer->start(header.to);
					} break;
					}
					fssize -= FrameHeader::size;
				} break;
				default: {
					fssize = 0; // error => stop handling
				} break;
				}
				frames = frames.right(fssize);
			}
		}
	}
}


void HPCProtocol::prepareDataToSend(const QByteArray &payload)
{
	if (this->socket) {
		if (this->txframe.size() == 0) {
			this->txframe = payload;
			this->sendTimer->start(HPCP_DELAY_SEND);
		} else if (this->txframe.size() + payload.size() <= HPCP_SENDBUF_SIZE_MAX) {
			this->txframe.append(payload); // by sendTimeUp or repeatTimeUp
		}
	}
}


void HPCProtocol::sendToPartner()
{
	if (this->socket && this->txframe.size()) {
		this->txframe.append((char)0); // end of the frame
		QByteArray payload = this->txframe;
		FrameHeader header (this->frameWr++, rData, 0, payload.size(), 0);
		this->txframe = header.toBytes() + payload;
		this->txSent = this->txframe.size();
		this->ackFailTimer->start(HPCP_ACKFAIL_TIMEOUT);
		this->socket->write(txframe);
	}
}


bool HPCProtocol::restart(QTcpSocket *socket)
{
	if (socket) {
		this->close();
		this->socket = socket;

		// send RST on disconnect instead FIN ( => nowait )
		auto fd = socket->socketDescriptor();
		struct linger lin = {
			.l_onoff = 1,
			.l_linger = 0
		};
		::setsockopt((int)fd, SOL_SOCKET, SO_LINGER, (const char *)&lin, sizeof(struct linger));

		m_partner.ip = socket->peerAddress().toString();
		m_partner.port = socket->peerPort();
		connect(socket, &QTcpSocket::readyRead, this, &HPCProtocol::read);
		connect(socket, &QTcpSocket::disconnected, this, &HPCProtocol::close);
		return true;
	}
	return false;
}


void HPCProtocol::close()
{
	if (socket) {
		socket->close();
		socket->deleteLater();
		socket = nullptr;
		txframe.clear();
		sendTimer->stop();
		ackFailTimer->stop();
		repeatTimer->stop();
		emit disconnected();
	}
}


HPCProtocol::HPCProtocol(QObject *parent) : QObject(parent)
{
	socket = nullptr;
	m_isBusy = false;
	m_isPrivileged = false;
	txSent = 0;
	frameWr = 0;

	sendTimer = new QTimer(this);
	sendTimer->setSingleShot(true);
	connect(sendTimer, &QTimer::timeout, this, &HPCProtocol::sendToPartner);

	ackFailTimer = new QTimer(this);
	ackFailTimer->setSingleShot(true);
	connect(ackFailTimer, &QTimer::timeout, this, &HPCProtocol::close);

	repeatTimer = new QTimer(this);
	repeatTimer->setSingleShot(true);
	connect(repeatTimer, &QTimer::timeout, this, &HPCProtocol::sendToPartner);

	const char *key256 = "kMJakpkdkyyLq1uUfuMj3wutCP4Q5wgA";
	const char *key128 = "CFBAEBD762763AB8";
	quint8 key[32];
	quint8 iv[16];
	for (int i = 0; i < 32; ++i) {
		key[i] = static_cast<quint8>(key256[i]) ^ 0x55;
		if (i < 16) {
			iv[i] = static_cast<quint8>(key128[i]) ^ 0xAA;
		}
	}

	// crypt
	ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

	qRegisterMetaType<HPCProtocol::Panel::Ident>("HPCProtocol::Panel::Ident");
	qRegisterMetaType<HPCProtocol::Panel::EthParams>("HPCProtocol::Panel::EthParams");
	qRegisterMetaType<HPCProtocol::Panel::State>("HPCProtocol::Panel::State");
	qRegisterMetaType<HPCProtocol::Panel::WifiParams>("HPCProtocol::Panel::WifiParams");
	qRegisterMetaType<HPCProtocol::Panel::ShortParams>("HPCProtocol::Panel::ShortParams");
	qRegisterMetaType<HPCProtocol::Panel::StrSysLog>("HPCProtocol::Panel::StrSysLog");
}


HPCProtocol::~HPCProtocol()
{
	qDebug() << "~HPCProtocol";
	EVP_CIPHER_CTX_free(ctx);
	// del socket? todo
}


void HPCProtocol::ctlSend(IdCmd cmd, const QByteArray &prm)
{
	QByteArray array;
	array.reserve(32);
	array.append((char)dCtl);
	array.append((char)cmd);
	if (prm.size())
		array.append(prm);
	else
		array.append((char)0);
	this->prepareDataToSend(array);
}


void HPCProtocol::setPanelIdent(const Panel::Ident &ident)
{
	this->m_panel.ident = ident;
	QByteArray array;
	array.reserve(128);
	array.append((char)dIdent);
	array.append(this->m_panel.ident.toBytes());
	this->prepareDataToSend(array);
}


void HPCProtocol::setPanelEth(const Panel::EthParams &eth)
{
	this->m_panel.eth = eth;
	QByteArray array;
	array.reserve(128);
	array.append((char)dEth);
	array.append(this->m_panel.eth.toBytes());
	this->prepareDataToSend(array);
}


void HPCProtocol::setPanelState(const Panel::State &state)
{
	this->m_panel.state = state;
	QByteArray array;
	array.reserve(128);
	array.append((char)dState);
	array.append((const char *)(&this->m_panel.state), 8);
	this->prepareDataToSend(array);
}


void HPCProtocol::setPanelWifi(const Panel::WifiParams &wifi)
{
	this->m_panel.wifi = wifi;
	QByteArray array;
	array.reserve(128);
	array.append((char)dWifi);
	array.append(this->m_panel.wifi.toBytes(this->ctx));
	this->prepareDataToSend(array);
}


void HPCProtocol::setPanelShort(const Panel::ShortParams &shortp)
{
	this->m_panel.shortp = shortp;
	QByteArray array;
	array.reserve(128);
	array.append((char)dShort);
	array.append(this->m_panel.shortp.toBytes());
	this->prepareDataToSend(array);
}


void HPCProtocol::setPanelStrSysLog(const Panel::StrSysLog &strSysLog)
{
	this->m_panel.strSysLog = strSysLog;
	QByteArray array;
	array.reserve(512);
	array.append((char)dStrSysLog);
	array.append(this->m_panel.strSysLog.toBytes());
	this->prepareDataToSend(array);
}


void HPCProtocol::setPanelStrUserLog(const Panel::StrUserLog &strUserLog)
{
	this->m_panel.strUserLog = strUserLog;
	QByteArray array;
	array.reserve(512);
	array.append((char)dStrUserLog);
	array.append(this->m_panel.strUserLog.toBytes());
	this->prepareDataToSend(array);
}


void HPCProtocol::setPanelUserPassword(const Panel::UserPassword &password)
{
    this->m_panel.userPassword = password;
    QByteArray array;
    array.reserve(128);
    array.append((char)dUserPassw);
    array.append(this->m_panel.userPassword.toBytes(this->ctx));
    this->prepareDataToSend(array);
}


QByteArray FrameHeader::toBytes()  const
{
	QByteArray ret(FrameHeader::size, 0);
	ret[0] = this->num;
	ret[1] = this->reason;
	ret[2] = this->to;
	ret[3] = (quint8)(this->dataSize >> 0);
	ret[4] = (quint8)(this->dataSize >> 8);
	ret[7] = this->ack;
	return ret;
}


void FrameHeader::fromBytes(const QByteArray &array)
{
	if (array.size() >= FrameHeader::size) {
		this->num = array[0];
		this->reason = array[1];
		this->to = array[2];
		this->dataSize = ((quint16)array[3] << 0) | ((quint16)array[4] << 8);
		this->reserve = 0;
		this->ack = array[7];
	}
}


HPCProtocol::Panel::Ident HPCProtocol::panelIdent() const
{
	return this->m_panel.ident;
}


HPCProtocol::Panel::EthParams HPCProtocol::panelEth() const
{
	return this->m_panel.eth;
}


HPCProtocol::Panel::State HPCProtocol::panelState() const
{
	return this->m_panel.state;
}


HPCProtocol::Panel::WifiParams HPCProtocol::panelWifi() const
{
	return this->m_panel.wifi;
}


HPCProtocol::Panel::ShortParams HPCProtocol::panelShort() const
{
	return this->m_panel.shortp;
}


HPCProtocol::Panel::StrSysLog HPCProtocol::panelStrSysLog() const
{
	return this->m_panel.strSysLog;
}


HPCProtocol::Panel::StrUserLog HPCProtocol::panelStrUserLog() const
{
	return this->m_panel.strUserLog;
}


HPCProtocol::Panel::UserPassword HPCProtocol::panelUserPassword() const
{
    return this->m_panel.userPassword;
}


HPCProtocol::FrameAddress HPCProtocol::partner() const
{
	return this->m_partner;
}


FrameHeader::FrameHeader(quint8 num, quint8 reason, quint8 to, quint16 dataSize, quint8 ack)
		: num(num), reason(reason), to(to), dataSize(dataSize), reserve(0), ack(ack)
{}


FrameHeader::FrameHeader(const QByteArray &array)
{
	this->num = 0;
	this->reason = 0;
	this->to = 0;
	this->dataSize = 0;
	this->reserve = 0;
	this->ack = 0;
	this->fromBytes(array);
}


QByteArray HPCProtocol::Panel::Ident::toBytes() const
{
	QByteArray ret;
	ret.reserve(128);
	ret.append(this->NameDisp);
	ret.push_back((char)0);
	ret.append(this->BuildVers);
	ret.push_back((char)0);
	ret.append(this->BootVers);
	ret.push_back((char)0);
	ret.append(this->ProjVers);
	ret.push_back((char)0);
	ret.append(this->MetaVers);
	ret.push_back((char)0);
	ret.append(this->KernelVers);
	ret.push_back((char)0);
	return ret;
}


int HPCProtocol::Panel::Ident::fromBytes(const quint8 *array)
{
	int it = 0;
	const char *str;
	for (int i = 0; i < 6; i++) {
		str = (const char *)array + it;
		it += strlen(str) + 1;
		switch (i) {
		case 0: this->NameDisp = str;
			break;
		case 1: this->BuildVers = str;
			break;
		case 2: this->BootVers = str;
			break;
		case 3: this->ProjVers = str;
			break;
		case 4: this->MetaVers = str;
			break;
		case 5: this->KernelVers = str;
			break;
		}
	}
	return it;
}


QByteArray HPCProtocol::Panel::EthParams::toBytes() const
{
	QByteArray ret;
	ret.reserve(128);
	ret.append(this->Mode);
	ret.append(this->IP);
	ret.push_back((char)0);
	ret.append(this->Mask);
	ret.push_back((char)0);
	ret.append(this->Gateway);
	ret.push_back((char)0);
	return ret;
}


int HPCProtocol::Panel::EthParams::fromBytes(const quint8 *array)
{
	int it = 0;
	this->Mode = (NetMode)array[it++];
	const char *str;
	for (int i = 0; i < 3; i++) {
		str = (const char *)array + it;
		it += strlen(str) + 1;
		switch (i) {
		case 0: this->IP = str;
			break;
		case 1: this->Mask = str;
			break;
		case 2: this->Gateway = str;
			break;
		}
	}
	return it;
}


QByteArray HPCProtocol::Panel::WifiParams::toBytes(EVP_CIPHER_CTX *ctx) const
{
	QByteArray ret, encrP;
	ret.reserve(128);
	ret.append(this->Mode);
	ret.append(this->IP);
	ret.push_back((char)0);
	ret.append(this->Mask);
	ret.push_back((char)0);
	ret.append(this->Gateway);
	ret.push_back((char)0);
	ret.append(this->Ssid);
	ret.push_back((char)0);
	ret.append(this->Bssid);
	ret.push_back((char)0);
	ret.append(this->Uname);
	ret.push_back((char)0);
	encryptString(ctx, this->Password, encrP);
	ret.append(encrP);
	ret.push_back((char)0);
	return ret;
}


int HPCProtocol::Panel::WifiParams::fromBytes(const quint8 *array, EVP_CIPHER_CTX *ctx)
{
	int it = 0;
	this->Mode = (NetMode)array[it++];
	const char *str;
	for (int i = 0; i < 5; i++) {
		str = (const char *)array + it;
		it += strlen(str) + 1;
		switch (i) {
		case 0: this->IP = str;
			break;
		case 1: this->Mask = str;
			break;
		case 2: this->Gateway = str;
			break;
		case 3: this->Ssid = str;
			break;
		case 4: this->Bssid = str;
			break;
		case 5: this->Uname = str;
			break;
		case 6:
			this->Password.clear();
			decryptString(ctx, str, this->Password);
			break;
		}
	}
	return it;
}



QByteArray HPCProtocol::Panel::ShortParams::toBytes() const
{
	QByteArray ret;
	ret.reserve(32);
	ret.push_back((char)this->netMode);
	ret.append((const char *)(&this->backlightTO), 2);
	ret.push_back((char)this->uiMode);
	ret.push_back((char)this->logFlags);
	return ret;
}


int HPCProtocol::Panel::ShortParams::fromBytes(const quint8 *array)
{
	int it = 0;
	this->netMode = (NetMode)array[it++];
	this->backlightTO = *((quint16*)(&array[it]));
	it += 2;
	this->uiMode = (UiMode)array[it++];
	this->logFlags = (LogFlags)array[it++];
	return it;
}


QByteArray HPCProtocol::Panel::StrSysLog::toBytes() const
{
	QByteArray ret;
	ret.reserve(256);
	ret.push_back((char)this->type);
	ret.append((const char *)(&this->timestamp), 8);
	ret.append(this->msg);
	ret.push_back((char)0);
	return ret;
}


int HPCProtocol::Panel::StrSysLog::fromBytes(const quint8 *array)
{
	int it = 0;
	this->type = (int)array[it++];
	this->timestamp = *((quint64*)(&array[it]));
	it += 8;
	this->msg = (const char *)(array + it);
	it += this->msg.length() + 1;
	return it;
}


QByteArray HPCProtocol::Panel::StrUserLog::toBytes() const
{
	QByteArray ret;
	ret.reserve(256);
	ret.append(this->fileName);
	ret.push_back((char)0);
	ret.append((const char *)(&this->line), 2);
	ret.append((const char *)(&this->timestamp), 8);
	ret.append(this->msg);
	ret.push_back((char)0);
	return ret;
}


int HPCProtocol::Panel::StrUserLog::fromBytes(const quint8 *array)
{
	int it = 0;
	this->fileName = (const char *)(array + it);
	it += this->fileName.length() + 1;
	this->line = *((quint16*)(&array[it]));
	it += 2;
	this->timestamp = *((quint64*)(&array[it]));
	it += 8;
	this->msg = (const char *)(array + it);
	it += this->msg.length() + 1;
	return it;
}


QByteArray HPCProtocol::Panel::UserPassword::toBytes(EVP_CIPHER_CTX *ctx) const
{
    QByteArray ret, encrP;
    encryptString(ctx, this->self, encrP);
    ret.append(encrP);
    ret.push_back((char)0);
    return ret;
}


int HPCProtocol::Panel::UserPassword::fromBytes(const quint8 *array, EVP_CIPHER_CTX *ctx)
{
    int it = 0;
    const char *str = (const char *)array;
    it += strlen(str) + 1;
    this->self.clear();
    decryptString(ctx, str, this->self);
    return it;
}


static int encrypt(EVP_CIPHER_CTX *ctx, const quint8 *plaintext, int plaintext_len, quint8 *ciphertext)
{
	if (!ctx || !plaintext || !plaintext_len || !ciphertext)
		return 0;

	int len;
	int ciphertext_len = 0;
	// encryption
	if (!EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
		return 0;
	}
	ciphertext_len = len;
	// padding
	if (!EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
		return 0;
	}
	ciphertext_len += len;
	return ciphertext_len;
}


static int decrypt(EVP_CIPHER_CTX *ctx, const quint8 *ciphertext, int ciphertext_len, quint8 *plaintext)
{
	if (!ctx || !ciphertext || !ciphertext_len || !plaintext)
		return 0;

	int len;
	int plaintext_len = 0;
	// decrypt
	if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
		return 0;
	}
	plaintext_len = len;
	// padding
	if (!EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
		return 0;
	}
	plaintext_len += len;
	return plaintext_len;
}


static bool base64Encode(const quint8 *binary, int size, QString &out)
{
	if (binary && size) {
		BIO *b64 = BIO_new(BIO_f_base64());
		if (b64) {
			BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
			BIO *sink = BIO_new(BIO_s_mem());
			if (sink) {
				BIO_push(b64, sink);
				BIO_write(b64, binary, size);
				BIO_flush(b64);
				char *encoded;
				BIO_get_mem_data(sink, &encoded);
				BIO_free_all(b64);
				out = encoded;
				return true;
			}
			BIO_free_all(b64);
		}
	}
	return false;
}


static bool base64Decode(const char* encoded, QByteArray &out)
{
	if (encoded && strlen(encoded)) {
		BIO *b64 = BIO_new(BIO_f_base64());
		if (b64) {
			BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
			BIO *source = BIO_new_mem_buf(encoded, -1);
			if (source) {
				BIO_push(b64, source);
				const int maxlen = strlen(encoded) / 4 * 3 + 1;
				QByteArray decoded(maxlen, 0);
				const int len = BIO_read(b64, decoded.data(), maxlen);
				decoded.resize(len);
				BIO_free_all(b64);
				out = decoded;
				return true;
			}
			BIO_free_all(b64);
		}
	}
	return false;
}


static bool encryptString(EVP_CIPHER_CTX *ctx, const QString &data, QByteArray &out)
{
	quint8 ciphertext[1024];
	int len = encrypt(ctx, (quint8 *)data.toStdString().c_str(), data.size(), ciphertext);
	if (len > 0) {
		QString encoded;
		if (base64Encode(ciphertext, len, encoded)) {
			out.clear();
			out.append(encoded);
			return true;
		}
	}
	return false;
}


static bool decryptString(EVP_CIPHER_CTX *ctx, const char *in, QString &out)
{
	QByteArray decoded;
	if (base64Decode(in, decoded)) {
		if (decoded.size() > 0) {
			quint8 ciphertext[1024];
			int len = decrypt(ctx, (quint8 *)decoded.data(), decoded.size(), ciphertext);
			if (len > 0) {
				out.clear();
				out.append((char *)ciphertext);
				return true;
			}
		}
	}
	return false;
}


bool operator!=(const HPCProtocol::FrameAddress &s1, const HPCProtocol::FrameAddress &s2)
{
	return !(s1.port == s2.port && s1.ip.compare(s2.ip) == 0);
}

