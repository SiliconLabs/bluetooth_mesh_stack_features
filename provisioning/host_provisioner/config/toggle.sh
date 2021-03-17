#!/bin/bash

if [ $# -eq 0 ];then
	cwd=$(cd `dirname $0`; pwd)
	fn=conf_full.xml
else
	fn=$1
	cwd=`pwd`
fi

echo "  --- Toggle the blacklist between 00 and 10 in ${cwd}/$fn ---  "

a=`grep -E "Blacklist.*10" ${cwd}/$fn`

# echo "$a"

if [ "$a" ]
then
  sed -i -r "s/Blacklist=\"10\"/Blacklist=\"00\"/g" ${cwd}/$fn
  echo "10 to 00"
else
  sed -i -r "s/Blacklist=\"00\"/Blacklist=\"10\"/g" ${cwd}/$fn
  echo "00 to 10"
fi
# cat ${cwd}/$fn | grep -i "blacklist"
exit 0
