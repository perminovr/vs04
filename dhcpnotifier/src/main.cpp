#include "dhcpnotifier.h"
#include <QtCore>
#include <QProcessEnvironment>
#include <getopt.h>
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>


static int Usage();


extern "C" {
	typedef struct option option_t;
}


int prefixToMask(int prefix, char *buf)
{
	if (prefix >= 0 && prefix <= 32) {
		uint32_t mask = prefix ? (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF : 0;
		mask = htonl(mask);
		return inet_ntop(AF_INET, &mask, buf, INET_ADDRSTRLEN)? 0 : -1;
	}
	return -1;
}


int main(int argc, char **argv)
{
	QString state;
	QString iface;
	QString ip;
	QString netmask;
	QString defgw;

	option_t loptions[] = {
		{"state", required_argument, 0, 's'},
		{"iface", required_argument, 0, 'i'},
		{"address", required_argument, 0, 'a'},
		{"mask", required_argument, 0, 'm'},
		{"route", required_argument, 0, 'r'},
		// {"daemon", no_argument, 0, 'd'},
		{0, 0, 0, 0}
	};

	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "s:i:a:m:r:", loptions, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 's': {
				state = QString(optarg);
			} break;
			case 'i': {
				iface = QString(optarg);
			} break;
			case 'a': {
				ip = QString(optarg);
				if (!ip.length())
					return Usage();
			} break;
			case 'm': {
				netmask = QString(optarg);
				if (netmask.length() > 0 &&netmask.length() <= 2) { // means prefix instead ip addr
					char buf[32];
					if ( prefixToMask(netmask.toInt(), buf) == 0 )
						netmask = buf;
					else
						return Usage();
				}
				if (!netmask.length())
					return Usage();
			} break;
			case 'r': {
				defgw = QString(optarg);
				if (!defgw.length())
					return Usage();
			} break;
			default: { } break;
		}
	}

	if (!state.length() || !iface.length()) {
		return Usage();
	}

	QCoreApplication app(argc, argv);

	IDhcpClient *provider = new DhcpNotifier(&app);

	QObject::connect(provider, &IDhcpClient::isReady, [&state, &provider, &iface, &ip, &netmask, &defgw](){
		bool add = (state == QStringLiteral("renew") || state == QStringLiteral("bound"));
		provider->notify(iface, ip, netmask, defgw, add);
	});

	QObject::connect(provider, &IDhcpClient::complete, [](){
		exit(0);
	});

	provider->start();
	return app.exec();
}


static int Usage()
{
	printf("%s",
			"Usage:\n" \
			"	-s|--state\n" \
			"	-i|--iface\n" \
			"	-a|--address\n" \
			"	-m|--mask\n" \
			"	-r|--route\n" \
	);
	return -1;
}
