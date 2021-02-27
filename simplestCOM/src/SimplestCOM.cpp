
#include "SimplestCOM.h"

#include <fstream>
#include <streambuf>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>



#define _MAX_FILE_SIZE 	(1 << 20)
#define sc_i(x) 		static_cast<int>(x)



#define _TO_CONSOLE(b) 						\
	(										\
		(32 <= b && b <= 126) 	/* chars */	\
		|| b == '\r' 		/* creturn */	\
		|| b == '\n' 		/* nline */		\
	)



using namespace std;
using namespace perri;



static void SigHandler(int sig);



void SimplestCOM::RdWork(void)
{
	char b;
	int res;

	while (1) {
		res = this->port->Read(&b);
		if (res == 1) {
			if ( _TO_CONSOLE(b) ) {
				cout << b;
			} else {
				this->HandleControlChars(b);
			}
			cout.flush();
		} else {
			usleep(1000);
		}
	}
}



void SimplestCOM::WrWork(void)
{
	string inp;
	inp.reserve(256);
	char c;

	switch (this->params.mode) {
	case InputMode::byLine:
		while (1) {
			inp.clear();
			getline(cin, inp);
			inp += "\r\n";
			this->SafePortWrite(&(inp[0]), inp.size());
		}
		break;
	case InputMode::byChars:
		this->SetCustomInputSettings();
		cout << this->hName << "Press ctrl+a to enter the menu" << endl;
		while (1) {
			c = getc(stdin);
			switch (c) {
				case 1: {
					if ( this->WrMenu() == MenuCode::abort )
						return;
				} break;
				default: {
					this->SendChar(c);
				} break;
			}
		}
		break;
	}
}



void SimplestCOM::SendChar(char c)
{
	static string str;
	str.reserve(8);
	str.clear();
	switch (c) {
	case '\n': 
		str = "\r\n";
		break;
	default:
		str.push_back(c);
		break;
	}
	this->SafePortWrite(str.c_str(), str.size());	
}



SimplestCOM::MenuCode SimplestCOM::ExitMenu(SimplestCOM::MenuCode code)
{
	this->file.clear();
	return code;
}



SimplestCOM::MenuCode SimplestCOM::WrMenu(void)
{
	char c = 'h';
	string path;

	while(1) {
		// reading below, first - output help
		switch (c) {
			case 'q': {
				cout << this->hName << "Exit program? [yn]" << endl;
				c = getc(stdin);
				if (c == 'y')
					return this->ExitMenu(MenuCode::abort);
			} break;
			case 'c': {
				cout << this->hName << "Exit menu? [yn]" << endl;
				c = getc(stdin);
				if (c == 'y')
					return this->ExitMenu(MenuCode::ret);
			} break;
			case 'l': {
				struct stat fstat;
				this->SetDefaultInputSettings();
				cout << this->hName << "Enter file-path: ";
				cout.flush();
				getline(cin, path);
				if ( stat(path.c_str(), &fstat) == 0 ) {
					if (fstat.st_size > _MAX_FILE_SIZE) {
						cout << this->hName << "Oversize: " << dec << fstat.st_size / 1024 << " KB (max 1 MB)" << endl;
					} else {
						ifstream ist(path.c_str());
						this->file.assign( (std::istreambuf_iterator<char>(ist)),
								std::istreambuf_iterator<char>() );
						cout << this->hName << "Read successfully" << endl;
					}
				} else {
					cout << this->hName << "Cannot get stat of " << path << endl;
				}
				this->SetCustomInputSettings();
			} break;
			case 't': {
				if ( this->file.size() > 0 ) {
					this->SafePortWrite(this->file.c_str(), this->file.size());
					cout << this->hName << "Sent" << endl;
				} else {
					cout << this->hName << "Please, load a file" << endl;
				}
			} break;
			case 'r': {
				vector <uint8_t> v(256);
				uint8_t start = 0;
				generate(v.begin(), v.end(), [&start]()->uint8_t {
					return start++;
				});
				this->SafePortWrite(&(v[0]), v.size());
				cout << this->hName << "Sent" << endl;
			} break;
			case 'h': {
				cout << endl << this->hName
						<< "Menu options:\n"
						<< "	l - load file\n"
						<< "	t - transfer file\n"
						<< "	r - transfer 256 raw chars\n"
						<< "	c - quit menu\n"
						<< "	q - quit prg\n"
						<< "	h - see that"
						<< endl;
			} break;
			default: {} break;
		}
		c = getc(stdin);
	}
}



void SimplestCOM::HandleControlChars(char b)
{
	if (this->params.iescape && b == '\x1b') {
		this->port->WaitSpace();
		if ( this->port->Read(&b) == 1 && b == '[')
			this->HandleIEscape();
	} else {
		cout << " 0x" << hex << sc_i(b);
	}
}



void SimplestCOM::HandleIEscape()
{
	string b = "\x1b[";
	b.resize(16);
	int res = this->port->ReadNonBlocking(&(b[2]), b.size()-2);
	b.resize(res+2);
	cout << b;
}



void SimplestCOM::Start()
{
	if (this->port->GetState() != COMPort::STATE::OPENED)
		return;
	this->rdThread->detach();
	this->WrWork();
}



void SimplestCOM::SetDefaultInputSettings(void) const
{
	tcsetattr(STDIN_FILENO, TCSANOW, &this->defStdinSet);
}



void SimplestCOM::SetCustomInputSettings(void) const
{
	tcflag_t flags = ICANON;
	flags |= this->params.echo? 0 : ECHO;
	flags = ~flags;

	struct termios stdinSet;
	tcgetattr(STDIN_FILENO, &stdinSet);
	stdinSet.c_lflag &= flags;
	stdinSet.c_cc[VTIME] = 0;
	stdinSet.c_cc[VMIN] = 1;
	tcsetattr(STDIN_FILENO, TCSANOW, &stdinSet);
}



/// default: portname = "USB0", baudrate = 9600
int SimplestCOM::Initialization(
		const std::string &portname,
		uint32_t baudrate)
{
	COMPort::InitializationStruct comInitStr;
	comInitStr.portName = portname;
	comInitStr.wireBaudRate = baudrate;
	comInitStr.additionalBaudRate = 0;
	comInitStr.timeOut.Ms = 20;
	comInitStr.timeOut.nChars = 0;

	this->port->Initialization(comInitStr);
	if (this->port->GetState() != COMPort::STATE::OPENED) {
		cout << this->hName << "COM init failed" << endl;
		return 0;
	} else {
		cout << this->hName << "COM started with"
				<< " name: " << portname
				<< " baud: " << baudrate << endl
				;
		return 1;
	}
}



void SimplestCOM::Notify(char code)
{
	this->SafePortWrite(&code, 1);
}



static perri::IPublisher <char> sigPublisher;



static void SigHandler(int sig)
{
	switch (sig) {
	case SIGINT: // ctrl+c		/
		sigPublisher.Publish('\x3');
		break;
	case SIGTSTP: // ctrl+z		/
		sigPublisher.Publish('\x1a');
		break;
	case SIGQUIT: // ctrl+\		/
		sigPublisher.Publish('\x1c');
		break;
	default:
		break;
	}
}



void SimplestCOM::SafePortWrite(const void *data, size_t size)
{
	this->wrMutex.lock();
	this->port->Write(data, size);
	this->wrMutex.unlock();
}



static struct termios GetDefSet()
{
	struct termios defStdinSet;
	tcgetattr(STDIN_FILENO, &defStdinSet);
	return defStdinSet;
}



SimplestCOM::SimplestCOM(const PrgParams &params)
		: params(params),
		  defStdinSet( GetDefSet() ),
		  hName("SimplestCOM::: "),
		  port(new COMPort()),
		  rdThread(new std::thread(&SimplestCOM::RdWork, this))
{
	switch (this->params.mode) {
	case InputMode::byLine:
		break;
	case InputMode::byChars:
		signal(SIGINT, SigHandler);
		signal(SIGTSTP, SigHandler);
		signal(SIGQUIT, SigHandler);
		break;
	}

	this->Subscribe(sigPublisher);
}



SimplestCOM::~SimplestCOM()
{
	this->SetDefaultInputSettings();
	delete port;
	delete rdThread;
}

