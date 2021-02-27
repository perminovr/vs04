TEMPLATE = subdirs
CONFIG += c++11
CONFIG += ordered

# lib
PRJLIBS = \
	common \
	platform \
	wdtctl \
	ssdp \
	idhcpclient \
	iwifistation \
	wpawifistation \
	networking \
	hminetworking \
	hmiprotocols \
	hmiplugin \
	qzip \
	hpcprotocol \

# exec
PRJEXEC = \
	hmi \
	signverify \
	wdtctl-exec \
	simplestCOM \
	dhcpnotifier \
	wifictl-test \

SUBDIRS += $${PRJLIBS} $${PRJEXEC}

#dhcpnotifier.depends = idhcpclient
#networking.depends = idhcpclient
#hminetworking.depends = networking
#hmiprotocols.depends = hminetworking
#hmiplugin.depends = hminetworking hmiprotocols
#hmi.depends = $${PRJLIBS}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
