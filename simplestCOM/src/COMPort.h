#ifndef COMPort_H
#define COMPort_H


#include <stdint.h>
#include <string>


#ifdef __linux__
	typedef uint32_t DWORD;
	typedef int HANDLE;
#else
	typedef unsigned long DWORD;
	typedef void* HANDLE;
#endif


namespace perri {

/*! ----------------------------------------------------------------------------------------
 * @brief: Interface between driver and hard devices
 * -----------------------------------------------------------------------------------------
 * */
class COMPort {
public:
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	enum class STATE {
		CLOSED = 0,
		OPENED
	};

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	struct TimeOutStruct {
		DWORD Ms;
		DWORD nChars;
	};

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 *
	 * NOTE: additionalBaudRate is used e.g. when the device is a radio-module with two BRs:
	 * 		- wire baud rate
	 * 		- air baud rate
	 * without additional baud rate this param must be equal to zero
	 * -------------------------------------------------------------------------------------
	 * */
	struct InitializationStruct {
		std::string portName;
		DWORD wireBaudRate;
		DWORD additionalBaudRate;
		TimeOutStruct timeOut;
	};

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	COMPort();
	COMPort(const InitializationStruct &initStr);

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	~COMPort();

	/*! ------------------------------------------------------------------------------------
	 * @brief: Opens the port
	 * -------------------------------------------------------------------------------------
	 * */
	COMPort::STATE Initialization(const InitializationStruct &initStr);

	/*! ------------------------------------------------------------------------------------
	 * @brief: Reopens the port if it was not be closed, but was be successfully initialized
	 * -------------------------------------------------------------------------------------
	 * */
	COMPort::STATE ReOpen();

	/*! ------------------------------------------------------------------------------------
	 * @brief: Closes the port
	 *
	 * NOTE: The port must be initialized again if it is needed after calling this method
	 * -------------------------------------------------------------------------------------
	 * */
	COMPort::STATE Close();

	/*! ------------------------------------------------------------------------------------
	 * @brief: Reads data from buffer (behavior is dependent on parameters of ChangeTimeOut);
	 * Returns received bytes (0:n) or failure (-1)
	 * -------------------------------------------------------------------------------------
	 * */
	int Read(void *buffer, size_t buffer_size = 1) const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Reads data from buffer without blocking;
	 * Returns received bytes (0:n) or failure (-1)
	 * -------------------------------------------------------------------------------------
	 * */
	int ReadNonBlocking(void *buffer, size_t buffer_size) const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Writes data to buffer (up to 256 bytes per write) / transfers data as is
	 * Returns transfered (n) bytes or failure (-1)
	 * -------------------------------------------------------------------------------------
	 * */
	int Write(const void *buffer, size_t buffer_size) const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Checks port for system errors
	 *
	 * return value is test result: true - test passed
	 * -------------------------------------------------------------------------------------
	 * */
	bool DoTest() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	COMPort::STATE GetState() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Flush by OS abilities
	 * -------------------------------------------------------------------------------------
	 * */
	bool FastFlush() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Slow flush: doing fast flush, gets delay = charsSpacing and tries to read
	 * byte. If last is failed when return true, else repeats from the beginning.
	 * -------------------------------------------------------------------------------------
	 * */
	void Flush() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	size_t GetAvailableBytesOfRecvBuf() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Sets timeOut
	 * -------------------------------------------------------------------------------------
	 * */
	void SetTimeOut(const TimeOutStruct &timeOut);

	/*! ------------------------------------------------------------------------------------
	 * @brief: Returns current time out
	 * -------------------------------------------------------------------------------------
	 * */
	const TimeOutStruct& GetCurrentTimeOut() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Waits until all characters of one transaction have arrived
	 * -------------------------------------------------------------------------------------
	 * */
	void WaitSpace() const;

private:
	/*! ------------------------------------------------------------------------------------
	 * @brief: File descriptor / used by OS
	 * -------------------------------------------------------------------------------------
	 * */
	HANDLE fd;

	/*! ------------------------------------------------------------------------------------
	 * @brief: File name / used by OS
	 * -------------------------------------------------------------------------------------
	 * */
	std::string fName;

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	COMPort::STATE state;

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	DWORD baudRate;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Current time out
	 * -------------------------------------------------------------------------------------
	 * */
	TimeOutStruct timeOut;

	/*! ------------------------------------------------------------------------------------
	 * @brief: length of 10 bytes in ms
	 * -------------------------------------------------------------------------------------
	 * */
	uint8_t charsSpacing;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Opens the port
	 * -------------------------------------------------------------------------------------
	 * */
	COMPort::STATE Open();

	/*! ------------------------------------------------------------------------------------
	 * @brief: Resets errno
	 * -------------------------------------------------------------------------------------
	 * */
	int GetLastSystemError() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Changes the algorithm for receiving data:
	 * User sets time out to read data and number of chars which should be received;
	 *
	 * NOTE: changes SYSTEM timeouts, not own "timeOut" variable
	 * -------------------------------------------------------------------------------------
	 * */
	bool ChangeTimeOut(const TimeOutStruct &timeOut) const;

};

}


#endif