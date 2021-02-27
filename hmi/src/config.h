﻿#ifndef CONFIG_H
#define CONFIG_H

#include "hmiplugin.h"

#define CONFIG_PLC_NAME "elsyma-vs04"

#if !PLC_EMU_BUILD
# define CONFIG_HOME_PATH	"/home/root/"
# define CONFIG_RUN_DIR		"/var/run/"
# define CONFIG_LOG_DIR		"/var/log/"
#else
# define CONFIG_HOST_USER	"roman"
# define CONFIG_HOME_PATH	"/home/" CONFIG_HOST_USER "/vs04-work/"
# define CONFIG_RUN_DIR		CONFIG_HOME_PATH "run/"
# define CONFIG_LOG_DIR		CONFIG_HOME_PATH "log/"
#endif

#define CONFIG_WORKING_DIR		CONFIG_HOME_PATH "WorkingDir/"
#define CONFIG_PARAMS_FILE_DIR  CONFIG_HOME_PATH "Conf/"
#define CONFIG_SHARE_DIR		CONFIG_HOME_PATH "Share/"

#define CONFIG_DEFAULT_USER_QML_FILE_NAME "Main.qml"

#define CONFIG_PARAMS_FILE_NAME			"service.cfg"
#define CONFIG_PARAMS_FILE_PATH			CONFIG_PARAMS_FILE_DIR CONFIG_PARAMS_FILE_NAME
#define CONFIG_PROTOCOLS_CFG_FILE_NAME	"protocols.cfg"
#define CONFIG_PROTOCOLS_CFG_FILE_PATH	CONFIG_PARAMS_FILE_DIR CONFIG_PROTOCOLS_CFG_FILE_NAME

#define CONFIG_BUILD_VER_FILE_NAME		CONFIG_SHARE_DIR "build.version"
#define CONFIG_BOOT_VER_FILE_NAME		CONFIG_SHARE_DIR "boot.version"
#define CONFIG_PROJECT_VER_FILE_NAME	CONFIG_SHARE_DIR "project.version"
#define CONFIG_META_VER_FILE_NAME		CONFIG_SHARE_DIR "meta.version"
#define CONFIG_KERNEL_VER_FILE_NAME		CONFIG_SHARE_DIR "kernel.version"

#define CONFIG_FTPD_INETD_CONF			CONFIG_RUN_DIR "inetd.conf"
#define CONFIG_FTPD_WORKING_DIR			CONFIG_RUN_DIR "ftpd-working-dir/"
#define CONFIG_FTPD_LOGIN				"ftphpcp"
#define CONFIG_FTPD_PASSWORD			"ftphpcp"
#define CONFIG_FTPD_PORT				11111
#define CONFIG_FTPD_ARCHIVE_NAME		"usercfg.zip"
#define CONFIG_FTPD_ARCHIVE_PATH		CONFIG_FTPD_WORKING_DIR CONFIG_FTPD_ARCHIVE_NAME
#define CONFIG_FTPD_ARCHIVE_SIZE_MAX	(32L*(0x100000))
#define CONFIG_BACKUPED_ARCHIVE_PATH	CONFIG_WORKING_DIR CONFIG_FTPD_ARCHIVE_NAME
#define CONFIG_FTPD_SYSUP_NAME			"sysup.zip"
#define CONFIG_FTPD_SYSUP_PATH			CONFIG_FTPD_WORKING_DIR CONFIG_FTPD_SYSUP_NAME

#define CONFIG_HPCP_PORT			22222

#define CONFIG_SYSLOG_FILE_NAME		"hmiworker.log"
#define CONFIG_SYSLOG_FILE_PATH		CONFIG_LOG_DIR CONFIG_SYSLOG_FILE_NAME
#define CONFIG_USERLOG_FILE_NAME	"hmiuser.log"
#define CONFIG_USERLOG_FILE_PATH	CONFIG_LOG_DIR CONFIG_USERLOG_FILE_NAME
#define CONFIG_LOG_SIZE_MAX			0x100000

#define CONFIG_QML_PACKAGE_NAME	 QML_PACKAGE_NAME

#define CONFIG_DEFAULT_QML_FILE_NAME	"Main.qml"

#define CONFIG_THREAD_STACK_SIZE	0x100000

#define CONFIG_WIFI_APN_LIST_MAX	10

#endif // CONFIG_H