#!/bin/bash

## ------------------------------------------
## DEK xdtthng
## 2016-04-22 Revision PA1
## ------------------------------------------

set -x
bundlename=$1
lib_path=/opt/ap/cphw/lib64

cmw_repo=`cmwea software-location-get`/coremw/repository

## if bundle directory does not exist then get out
[ ! -d $cmw_repo/$bundlename ]  && { exit 1; }

rpmpackage=`cat $cmw_repo/$bundlename/rpm_list.txt | awk '{print $1}'`
#echo "rpmpackage $rpmpackage"

# Extract libCXC*.txt files from the bundle
rpm_content=`rpm -qp $cmw_repo/$bundlename/$rpmpackage --filesbypkg | awk -F"/" '{print $NF}' | grep ".txt"`
#echo "rpm content $rpm_content"

found=1
for libtxt in ${lib_path}/libCXC106*.txt
do
	file=`basename $libtxt`
	##echo "$file"
	
	n=`echo $rpm_content | grep $file | wc -l`
	#echo "number is $n"
	
	if [ $n -eq 0 ]
	then
		found=0;
		break;
	fi
done

[ $found -eq 0 ] && { exit 1; }
exit 0
