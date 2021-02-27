/*******************************************************************************
  * @file	wdtctl.c
  * @author  Перминов Р.И.
  * @version v0.0.0.1
  * @date	30.05.2019
  *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <string.h>
#include <fcntl.h>
#include <linux/watchdog.h>

#include "wdtctl.h"



#define WDTCTL_DAEMON_EV_SIZE		2
#define WDTCTL_DAEMON_PIPE_EV		1
#define WDTCTL_DAEMON_STATUS_EV		0
#define WDTCTL_DAEMON_PIPE_FD		ev[WDTCTL_DAEMON_PIPE_EV].data.fd
#define WDTCTL_DAEMON_STATUS_FD		ev[WDTCTL_DAEMON_STATUS_EV].data.fd



static int _wdtctl_fd = -1;



/*! ---------------------------------------------------------------------------
 * @fn: wdtctl_settiomeout
 *
 * @brief: Установка таймаута до сброса без пинга
 *
 * input parameters
 * @param timeout - время до сброса в c
 *
 * output parameters
 *
 * return
 * 		== 0 - успешно
 * 		!= 0 - провал
*/
int wdtctl_settiomeout(int timeout)
{
	int fd = open(WDTCTL_PIPE_NAME, O_WRONLY);
	char buf[] = "t\1";
	buf[1] = (char)timeout;
	int res = write(fd, buf, 2);
	close(fd);
	return (res <= 0)? -1 : 0;
}



/*! ---------------------------------------------------------------------------
 * @fn: wdtctl_ping
 *
 * @brief: Пнуть wdt
 *
 * input parameters
 *
 * output parameters
 *
 * return
 * 		== 0 - успешно
 * 		!= 0 - провал
*/
int wdtctl_ping(void)
{
	int fd = open(WDTCTL_PIPE_NAME, O_WRONLY);
	int res = write(fd, "p", 1);
	close(fd);
	return (res <= 0)? -1 : 0;
}



static int _wdtctl_open_fd()
{
	if (_wdtctl_fd <= 0) {
		_wdtctl_fd = open(WDTCTL_DEV_NAME, O_WRONLY);
		if (_wdtctl_fd <= 0) {
			return -1;
		}
	}
	return 0;
}



static int _wdtctl_read_status()
{
	int ret = -1;
	char pin_str[64];
	sprintf(pin_str, "/sys/class/gpio/gpio%d/value", WDTCTL_STATUS_PIN);
	int fd = open(pin_str, O_RDONLY);
	char val = 0;
	if ( read(fd, &val, 1) == 1 ) {
		ret = (val == '0')? 0 : 1;
	}
	if (fd > 0)
		close(fd);
	return ret;
}



int _wdtctl_ping(void)
{
	int val = _wdtctl_read_status();
	if (val < 0)
		return -2;

	if (!val) { // wdt is not blocked
		if ( _wdtctl_open_fd() )
			return -1;

		if ( write(_wdtctl_fd, "1", 1) <= 0 )
			return -3;

		puts("Ping WDT done");
	}

	return val;
}



static int _wdtctl_enable(void)
{
	return _wdtctl_ping();
}



static int _wdtctl_disable(void)
{
	if ( _wdtctl_open_fd() )
		return -1;

	if ( write(_wdtctl_fd, "V", 1) <= 0 )
		return -2;

	puts("Disable WDT done");

	// если не закрыть, то не выключится
	close(_wdtctl_fd);
	_wdtctl_fd = -1;

	return 0;
}



int _wdtctl_settiomeout(int timeout)
{
	if ( _wdtctl_open_fd() )
		return -1;

	if ( ioctl(_wdtctl_fd, WDIOC_SETTIMEOUT, &timeout) )
		return -3;

	puts("Set timeout WDT done");

	return 0;
}



static int _wdtctl_daemon(void)
{
	unlink(WDTCTL_PIPE_NAME);
	if ( mkfifo(WDTCTL_PIPE_NAME, S_IRUSR | S_IWUSR) )
		return -1;

	struct epoll_event ev[WDTCTL_DAEMON_EV_SIZE] = {0};

	char pin_str[64];
	sprintf(pin_str, "/sys/class/gpio/gpio%d/value", WDTCTL_STATUS_PIN);

	ev[WDTCTL_DAEMON_PIPE_EV].events = EPOLLIN;
	ev[WDTCTL_DAEMON_PIPE_EV].data.fd = open(WDTCTL_PIPE_NAME, O_RDWR);
	ev[WDTCTL_DAEMON_STATUS_EV].events = EPOLLET;
	ev[WDTCTL_DAEMON_STATUS_EV].data.fd = open(pin_str, O_RDONLY);

	if (WDTCTL_DAEMON_PIPE_FD < 0 || WDTCTL_DAEMON_STATUS_FD < 0)
		return -2;

	int epfd = epoll_create(1);
	if ( epfd < 0 )
		return -3;

	epoll_ctl(epfd, EPOLL_CTL_ADD, WDTCTL_DAEMON_PIPE_FD, &ev[WDTCTL_DAEMON_PIPE_EV]);
	epoll_ctl(epfd, EPOLL_CTL_ADD, WDTCTL_DAEMON_STATUS_FD, &ev[WDTCTL_DAEMON_STATUS_EV]);

	int block = 0;
	int wdtwork = 0;
	struct epoll_event events;
	for (;;) {
		epoll_wait(epfd, &events, 1, -1);
		if (events.data.fd == WDTCTL_DAEMON_PIPE_FD) {
			char c;
			if ( read (WDTCTL_DAEMON_PIPE_FD, &c, 1) != 1 )
				return -4;
			printf ("cmd=%c\n", c);
			switch (c) {
				case 'e': {
					block = 0;
					if ( !_wdtctl_enable() )
						wdtwork = 1;
				} break;
				case 'd': {
					block = 1;
					if ( !_wdtctl_disable() )
						wdtwork = 0;
				} break;
				case 'p': {
					if ( !_wdtctl_ping() )
						wdtwork = 1;
				} break;
				case 't': {
					if ( read (WDTCTL_DAEMON_PIPE_FD, &c, 1) == 1 ) {
						int timeout = (int)c;
						printf ("timeout=%d\n", timeout);
						_wdtctl_settiomeout(timeout);
						if (wdtwork)
							_wdtctl_enable();
						else
							_wdtctl_disable();
					}
				} break;
				default: break;
			}
		}
		if (events.data.fd == WDTCTL_DAEMON_STATUS_FD) {
			int val = _wdtctl_read_status();
			printf ("status=%d, block=%d\n", val, block);
			if (!block) {
				switch (val) {
					case 0: {
						if ( !_wdtctl_enable() )
							wdtwork = 1;
					} break;
					case 1: {
						if ( !_wdtctl_disable() )
							wdtwork = 0;
					} break;
					default: break;
				}
			}
		}
	}

	return 0;
}



static int Usage();



int main(int argc, char **argv)
{
	struct option loptions[] = {
		{"daemon", no_argument, 0, 'd'},
		{"enable", no_argument, 0, 'e'},
		{"disable", no_argument, 0, 's'},
		{"ping", no_argument, 0, 'p'},
		{"timeout", required_argument, 0, 't'},
	  	{0, 0, 0, 0}
	};
	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "despt:", loptions, &option_index);
		if (c == -1)
   				break;

		switch (c) {
			case 'd': {
				return _wdtctl_daemon();
			} break;
			case 'e': {
				int fd = open(WDTCTL_PIPE_NAME, O_WRONLY);
				int res = write(fd, "e", 1);
				return (res <= 0)? -1 : 0;
			} break;
			case 's': {
				int fd = open(WDTCTL_PIPE_NAME, O_WRONLY);
				int res = write(fd, "d", 1);
				return (res <= 0)? -1 : 0;
			} break;
			case 'p': {
				int fd = open(WDTCTL_PIPE_NAME, O_WRONLY);
				int res = write(fd, "p", 1);
				return (res <= 0)? -1 : 0;
			} break;
			case 't': {
				int fd = open(WDTCTL_PIPE_NAME, O_WRONLY);
				char buf[] = "t\1";
				char timeout;
				int res = -1;
				if ( sscanf(optarg, "%hhu", &timeout) == 1) {
					buf[1] = timeout;
					res = write(fd, buf, 2);
				}
				return (res <= 0)? -1 : 0;
			} break;
			default: break;
		}
	}

	return Usage();
}



static int Usage()
{
	printf("%s",
			"Usage:\n" \
			"	-e|--enable\n" \
			"	-s|--disable\n" \
			"	-d|--daemon\n" \
			"	-t|--timeout TIME\n" \
			"	-p|--ping\n" \
	);
	return -1;
}
