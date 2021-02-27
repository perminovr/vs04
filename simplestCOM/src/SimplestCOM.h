#ifndef SIMPLESTCOM_H_
#define SIMPLESTCOM_H_

#include <thread>
#include <mutex>
#include <cstdint>

#include <string>
#include <vector>
#include <termios.h>

#include "COMPort.h"
#include "Observer.h"



class SimplestCOM : public perri::ISubscriber <char> {
public:
	enum InputMode {
		byLine = 0,
		byChars = 1,
	};
	struct PrgParams {
		InputMode mode : 1;
		int echo : 1;
		int iescape : 1;
	};

	SimplestCOM(const PrgParams &params);
	~SimplestCOM();
	int Initialization(
			const std::string &portname = "USB0",
			uint32_t baudrate = 9600);
	void Start();

private:
	const PrgParams params;
	const struct termios defStdinSet;
	const std::string hName;
	perri::COMPort *port;
	std::thread *rdThread;

	std::string file;
	std::mutex wrMutex;

	enum MenuCode {
		abort,
		ret
	};

	void RdWork();
	void WrWork();
	MenuCode WrMenu();
	MenuCode ExitMenu(MenuCode code);
	void HandleControlChars(char b);
	void HandleIEscape();
	void SetDefaultInputSettings() const;
	void SetCustomInputSettings() const;
	void SafePortWrite(const void *data, size_t size);
	void SendChar(char c);

	void Notify(char code) override;
};



#endif /* SIMPLESTCOM_H_ */
