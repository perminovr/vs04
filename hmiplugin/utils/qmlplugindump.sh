# 1 - qmake full path
# 2 - pkgname
# 3 - pkgver major.minor
# 4 - importpath
# 5 - libs path

if [ -z $1 -o -z $2 -o -z $3 -o -z $4 ]; then
	echo "1=$1 2=$2 3=$3 4=$4 : arg empty"
	exit 1
fi

if [ -x $1 ]; then
	BINS=`dirname $1`
	DUMP=$BINS/qmlplugindump

	if [ -x $DUMP ]; then
		mkdir -p $4/$2
		echo $DUMP
		(
		export QT_IM_MODULE="qtvirtualkeyboard"
		export LD_LIBRARY_PATH=$5
		$DUMP -nonrelocatable $2 $3 $4 > $4/$2/plugins.qmltypes
		)
		echo "qmlplugindump done"
		exit 0
	else
		echo "qmlplugindump not found"
		exit 2 # warn
	fi
else
	echo "$1 is not providing qmake path"
	exit 1
fi

exit 0
