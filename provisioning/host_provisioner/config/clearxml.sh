#!/bin/bash

if [ $# -eq 0 ];then
	cwd=$(cd `dirname $0`; pwd)
	fn=conf.xml
else
	fn=$1
	cwd=`pwd`
fi

echo "Clearing all done|blacklist|err flags in file - ${cwd}/$fn"
# sed -i "s/done=\"01\"/done=\"00\"/g" ${cwd}/$fn
# sed -i "s/done=\"11\"/done=\"00\"/g" ${cwd}/$fn
sed -i -r "s/done=\"[0-9a-fA-F]+\"/done=\"00\"/g" ${cwd}/$fn
sed -i -r "s/err=\"[0-9a-fA-F]+\"/err=\"00000000\"/g" ${cwd}/$fn
sed -i -r "s/blacklist=\"[0-9a-fA-F]+\"/blacklist=\"00\"/g" ${cwd}/$fn
sed -i -r "s/addr=\"[0-9a-fA-F]+\"/addr=\"00\"/g" ${cwd}/$fn
cat ${cwd}/$fn | grep done
