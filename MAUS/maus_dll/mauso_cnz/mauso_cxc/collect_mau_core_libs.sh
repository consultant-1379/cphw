#!/bin/bash
mausobin_cphw_path_apz25='/app/APG43L/SDK/CP/HWStage/APZ25/CPUB/MAUS'
mausobin_cphw_path_apz26='/app/APG43L/SDK/CP/HWStage/APZ26/CPUB/MAUS'
mausobin_ver=$1
cxc_path=$2
lib_ext=${cxc_path}/bin/lib_ext

. ${cxc_path}/mau_core_versions.sh

set -x

function process_mau_core_lib
{
    mauslib_ver=$1  
    mauslib_cphw_dll=`find -L $mausobin_cphw_path_apz25 $mausobin_cphw_path_apz26 -name '*.txt' | xargs grep -w $mauslib_ver | xargs -n1 dirname`

    for i in ${mauslib_cphw_dll}/lib*
    do
    	cp -f $i "${lib_ext}/."
    done
    
    for i in ${mauslib_cphw_dll}/mibs*
    do
    	cp -f $i "${lib_ext}/."
    done
}


function log_note
{
	note_file=${cxc_path}/build_notes.txt
	
	mauso_ver="MAUSOBIN Version = $mausobin_ver"
	mau_core_ver="MAU core Version = $MAU_CORE_VERS"
	n_lines=`grep "$mauso_ver" $note_file | wc -l`
	
	if [ $n_lines -eq 0 ]
	then
		echo $mauso_ver >> $note_file
		echo $mau_core_ver >> $note_file
		echo " "  >> $note_file
	fi

}

##############
#  main      #
##############

log_note

for i in $MAU_CORE_VERS 
do

   process_mau_core_lib $i
   
done
