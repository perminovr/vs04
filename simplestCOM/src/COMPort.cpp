
#include "COMPort.h"
#include <string>

#ifdef __linux__
	#include <errno.h>
	#include <fcntl.h>
	#include <termios.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/ioctl.h>
#else
	//#define UNICODE
	#include <windows.h>
#endif


using namespace perri;



COMPort::COMPort()
{
	this->state = COMPort::STATE::CLOSED;
}



COMPort::COMPort(const InitializationStruct &initStr)
{
	this->Initialization(initStr);
}



COMPort::STATE COMPort::Initialization(const COMPort::InitializationStruct &initStr)
{
	this->state = COMPort::STATE::CLOSED;
	//
	this->baudRate = initStr.wireBaudRate;
	this->timeOut.Ms = initStr.timeOut.Ms;
	this->timeOut.nChars = initStr.timeOut.nChars;
	DWORD tmp_BR =
			(initStr.additionalBaudRate != 0 &&
			initStr.wireBaudRate > initStr.additionalBaudRate)?
			initStr.additionalBaudRate : initStr.wireBaudRate;
	this->charsSpacing = static_cast <uint8_t> (
		(8000.0 / static_cast <double> (tmp_BR)) * 10.0
	);
	this->charsSpacing += this->charsSpacing? 0 : 1;
#ifdef __linux__
	this->fName = "/dev/tty" + initStr.portName;
#else
	this->fName = initStr.portName;
#endif
	this->Open();


	return this->state;
}



COMPort::~COMPort()
{
	this->Close();
}



const COMPort::TimeOutStruct& COMPort::GetCurrentTimeOut() const
{
	return this->timeOut;
}



size_t COMPort::GetAvailableBytesOfRecvBuf() const
{
#ifdef __linux__
	size_t buf_size;
	ioctl(this->fd, FIONREAD, &buf_size);
	return buf_size;
#else
	/* WINDOWS AVAILABLE BYTES */
	return 0;
#endif
}



COMPort::STATE COMPort::Open()
{
#ifdef __linux__
	this->fd = open(this->fName.c_str(), O_RDWR | O_NOCTTY);
	if (this->fd != -1) {
		termios options;
		tcgetattr(this->fd, &options);

		switch (this->baudRate) {
		case 300: this->baudRate = B300; break;
		case 600: this->baudRate = B600; break;
		case 1200: this->baudRate = B1200; break;
		case 2400: this->baudRate = B2400; break;
		case 4800: this->baudRate = B4800; break;
		case 9600: this->baudRate = B9600; break;
		case 19200: this->baudRate = B19200; break;
		case 38400: this->baudRate = B38400; break;
		case 57600: this->baudRate = B57600; break;
		case 115200: this->baudRate = B115200; break;
		case 230400: this->baudRate = B230400; break;
		case 460800: this->baudRate = B460800; break;
		case 921600: this->baudRate = B921600; break;
		case 1152000: this->baudRate = B1152000; break;
		case 2000000: this->baudRate = B2000000; break;
		case 2500000: this->baudRate = B2500000; break;
		case 3000000: this->baudRate = B3000000; break;
		case 4000000: this->baudRate = B4000000; break;
		default: this->baudRate = B9600; break;
		}

		options.c_cflag = 0;
		cfsetispeed(&options, this->baudRate);
		cfsetospeed(&options, this->baudRate);
		options.c_cflag |= CS8 | CREAD | CLOCAL;
		//
		options.c_iflag = 0;
		//
		options.c_oflag = 0;
		//
		options.c_lflag = 0;
		//
		options.c_cc[VMIN] = 0;
		options.c_cc[VTIME] = 0;

		int res = tcsetattr(this->fd, TCSAFLUSH, &options);

		if (res != -1) {
			this->state = COMPort::STATE::OPENED;
		}
	}
#else
	std::wstring wstemp = std::wstring(this->fName.begin(), this->fName.end());
	LPCWSTR lpfName = wstemp.c_str();

	// FileName, AccessFlags, ShareMode, SecurityAttributes,
	//		CreationDisposition, FlagsAndAttributes, hTemplateFile
	// COMname, w/r, file-capture, default access,
	//		open existing file, default file attr, ignore when using existing file
	this->fd = CreateFileW(lpfName, GENERIC_READ | GENERIC_WRITE, 0, 0,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (this->fd != INVALID_HANDLE_VALUE) {
		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (GetCommState(this->fd, &dcbSerialParams)) {
			dcbSerialParams.BaudRate = this->baudRate;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

			if (SetCommState(this->fd, &dcbSerialParams)) {
				this->state = COMPort::STATE::OPENED;
			}
		}
	}
#endif
	return this->state;
}



COMPort::STATE COMPort::ReOpen()
{
	if (this->state != COMPort::STATE::CLOSED) {
		this->Close();
		this->Open();
	}
	return this->state;
}



COMPort::STATE COMPort::Close()
{
	if (this->state != COMPort::STATE::CLOSED) {
		this->state = COMPort::STATE::CLOSED;
#ifdef __linux__
		close(this->fd);
#else
		CloseHandle(this->fd);
#endif
	}
	return this->state;
}



COMPort::STATE COMPort::GetState() const
{
	return this->state;
}



void COMPort::SetTimeOut(const TimeOutStruct &timeOut)
{
	this->timeOut = timeOut;
}



bool COMPort::ChangeTimeOut(const TimeOutStruct &timeOut) const
{
#ifdef __linux__
	termios options;
	tcgetattr(this->fd, &options);
	if (options.c_cc[VMIN] == timeOut.nChars && options.c_cc[VTIME] == timeOut.Ms / 100)
		return 1;
	options.c_cc[VMIN] = timeOut.nChars;
	options.c_cc[VTIME] = timeOut.Ms / 100;
	int res = tcsetattr(this->fd, TCSADRAIN, &options);
	return (res != -1);
#else
	COMMTIMEOUTS CommTimeOuts;
	DWORD res;
	if (GetCommTimeouts(this->fd, &CommTimeOuts)) {

		/*CommTimeOuts.ReadIntervalTimeout = timeOut;
		CommTimeOuts.ReadTotalTimeoutMultiplier = ((this->charsSpacing / 8) + 1);
		CommTimeOuts.ReadTotalTimeoutConstant = timeOut;
		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		CommTimeOuts.WriteTotalTimeoutConstant = 0;*/
		CommTimeOuts.ReadIntervalTimeout = this->charsSpacing;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		CommTimeOuts.ReadTotalTimeoutConstant = timeOut.Ms;
		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		CommTimeOuts.WriteTotalTimeoutConstant = 0;

		res = SetCommTimeouts(this->fd, &CommTimeOuts);
	} else {
		res = 0;
	}
	return (res != 0);
#endif
}



void COMPort::Flush() const
{
	uint8_t buf;
	int res;
	TimeOutStruct TOStr;
	TOStr.Ms = 1;
	TOStr.nChars = 0;
	this->ChangeTimeOut(TOStr);
	while (1) {
		this->FastFlush();
#ifdef __linux__
		usleep(this->charsSpacing * 1000);
		res = read(this->fd, &buf, 1);
#else
		DWORD bytes_recvd;
		Sleep(this->charsSpacing);
		this->ChangeTimeOut(this->timeOut);
		ReadFile(this->fd, &buf, 1, &bytes_recvd, 0);
		res = static_cast <int> (bytes_recvd);
#endif
		if (res == 0)
			break;
	}
	this->ChangeTimeOut(this->timeOut);
	return;
}



bool COMPort::FastFlush() const
{
#ifdef __linux__
	int res = tcflush(this->fd, TCIFLUSH);
	return (res != -1);
#else
	DWORD res = PurgeComm(this->fd, PURGE_RXCLEAR);
	return (res != 0);
#endif
}



bool COMPort::DoTest() const
{
	int lastErr;
#ifdef __linux__
	termios options;
	errno = 0;
	tcgetattr(this->fd, &options);
	lastErr = this->GetLastSystemError();
	return (lastErr == 0);
#else
	DCB dcbSerialParams = {0};
	SetLastError(0);
	GetCommState(this->fd, &dcbSerialParams);
	lastErr = this->GetLastSystemError();
	return (lastErr == NO_ERROR || \
		lastErr == ERROR_MORE_DATA || \
		lastErr == WAIT_TIMEOUT);
#endif
}



int COMPort::GetLastSystemError() const
{
	int lastErr = 0;
#ifdef __linux__
	if (errno != 0) {
		lastErr = errno;
	}
	errno = 0;
#else
	if (GetLastError() != 0) {
		lastErr = GetLastError();
	}
	SetLastError(0);
#endif
	return lastErr;
}



void COMPort::WaitSpace() const
{
	size_t available = this->GetAvailableBytesOfRecvBuf(), tmp;
	while (1) {
#ifdef __linux__
		usleep(this->charsSpacing * 1000);
#else
		Sleep(this->charsSpacing);
#endif
		tmp = this->GetAvailableBytesOfRecvBuf();
		if (available != tmp) {
			available = tmp;
		} else {
			break;
		}
	}
	return;
}



/// default: buf_size = 1
int COMPort::Read(void *buf, size_t buf_size) const
{
	int res;
#ifdef __linux__
	TimeOutStruct TOStr;
	timeval tv;
	fd_set rfds;
	//
	tv.tv_sec = this->timeOut.Ms / 1000;
	tv.tv_usec = (this->timeOut.Ms % 1000) * 1000;
	TOStr.Ms = 0;
	TOStr.nChars = buf_size;
	//
	// should be VT = 0 and VM > 0 for RM !!!
	this->ChangeTimeOut(TOStr);
	//
	FD_ZERO(&rfds);
	FD_SET(this->fd, &rfds);
	res = select(this->fd + 1, &rfds, NULL, NULL, &tv);
	//
	if (res > 0)
		res = read(this->fd, buf, buf_size);
	else
		this->FastFlush();
#else
	DWORD bytes_recvd;
	this->ChangeTimeOut(this->timeOut);
	res = ReadFile(this->fd, buf, buf_size, &bytes_recvd, 0);
	res = (res != 0)? static_cast <int> (bytes_recvd) : -1;
#endif
	return res;
}



int COMPort::ReadNonBlocking(void *buffer, size_t buffer_size) const
{
	int res;
#ifdef __linux__
	TimeOutStruct TOStr {0, 0};
	this->ChangeTimeOut(TOStr);
	res = read(this->fd, buffer, buffer_size);
#else
#endif
	return res;
}



int COMPort::Write(const void *buf, size_t buf_size) const
{
	int res;
#ifdef __linux__
	res = write(this->fd, buf, buf_size);
#else
	DWORD bytes_written;
	res = WriteFile(this->fd, buf, buf_size, &bytes_written, 0);
	res = (res != 0)? static_cast <int> (bytes_written) : -1;
#endif
	return res;
}
