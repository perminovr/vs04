LIBTARGET = hmiprotocols
BASEDIR   = $${PWD}
INCLUDEPATH *= $${BASEDIR}/include $${BASEDIR}/include/client $${BASEDIR}/include/server
LIBS += -l$${LIBTARGET}