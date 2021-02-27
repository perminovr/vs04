
#include <getopt.h>
#include <string>
#include <sstream>
#include <iostream>

#include "SimplestCOM.h"



using std::string;
using std::ostringstream;
using std::exception;
using std::cout;
using std::endl;
using std::cin;



static void Usage()
{
	cout << "Usage:" << endl
			<< "	[-b|--baud X]" << endl
			<< "		supported baudrates: 300, 600, ... up to 38400, " << endl
			<< "		57600, ... up to 921600" << endl
			<< "		additional: 1152000, 2000000, 2.5M, 3M, 4M" << endl
			<< "		default: 9600" << endl
			<< "	[-p|--port portname]" << endl
			<< "		\"portname\" - name without \"/dev/tty\", " << endl
			<< "		e.g: /dev/ttyS1 -> \'-p S1\'" << endl
			<< "		default: USB0" << endl
			<< "	[--by-chars]" << endl
			<< "		with menu-features" << endl
			<< "	[--by-lines] (*)" << endl
			<< "		without menu-features" << endl
			<< "	[--echo]" << endl
			<< "	[--no-echo] (*)" << endl
//			<< "	[--escape-in]" << endl
//			<< "		adds del, arrows, etc hard-echo-features" << endl
//			<< "	[--no-escape-in] (*)" << endl
			<< "	[-h|--help]" << endl
			;
	exit(1);
}



int main(int argc, char **argv)
{
	SimplestCOM *prg;
	string portname = "USB0";
	uint32_t baudrate = 9600;
	int _inputMode = 0;
	int _echo = 0;
	int _iescape = 0;
	{
		ostringstream stringStream;
		struct option loptions[] = {
			{"port", required_argument, 0, 'p'},
			{"baud", required_argument, 0, 'b'},
			{"by-chars", no_argument, &_inputMode, 1},
			{"by-lines", no_argument, &_inputMode, 0},
			{"echo", no_argument, &_echo, 1},
			{"no-echo", no_argument, &_echo, 0},
//			{"escape-in", no_argument, &_iescape, 1},
//			{"no-escape-in", no_argument, &_iescape, 0},
			{"help", no_argument, 0, 'h' },
		  	{0, 0, 0, 0}
		};
		while (1) {
			int option_index = 0;
			int c = getopt_long(argc, argv, "p:b:h", loptions, &option_index);
			if (c == -1)
	   				break;

			stringStream.clear();
			stringStream.str("");
			string tmp;
			switch (c) {
				case 0:
					break;
				case 'p':
					stringStream << optarg;
					portname = stringStream.str();
					break;
				case 'b':
					stringStream << optarg;
					tmp = stringStream.str();
					try {
						baudrate = stoi(tmp);
						break;
					} catch (exception &ex) {
						cout << endl << "Error baud value" << endl << endl;
					} // no break;
				default:
					Usage();
					break;
			}
		}
	}

	prg = new SimplestCOM({
		static_cast <SimplestCOM::InputMode> (_inputMode),
		_echo,
		_iescape
	});
	if ( prg->Initialization(portname, baudrate) )
		prg->Start();
	delete prg;

	return 0;
}
