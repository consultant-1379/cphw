#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       createUPCampaign.sh
# Description:
#       <script_functionality_description>
# Note:
#       <script_notes>
##
# Usage:
#       ./createUpCampaign.sh 
##
# Output:
#       <script_output_description>
##
# Changelog:
#
# 2014-06-20 xfabpag Fixed to handle the naming convention of HA AGENT
# 2014-04-30 xfabpag Fixed to support also the installation of new NORED blocks
# 2014-04-28 xfabpag UP.conf parameters aligned to 19/102 60-LZY 214 5161 Uen PB13
# 2014-04-24 xfabpag Adapted to the new format of blocks_base file described in 19/102 60-LZY 214 5161 Uen PB13
# 2014-04-23 xfabpag CXC number read from makefile (not from sources.list) for BUP (option -t)
# 2014-02-24 eanbuon New baseline functionality
# 2014-02-04 xfabpag Bug fixing
# 2014-02-03 xfabpag Removed no more used code
# 2014-01-30 xfabpag Bug fixing
#                    Removed update.list checks if -f option is used
#                    Installation-related sections skipped if -f option is not used
# 2014-01-28/29 xclaatt/xfabpag Bug fixing
# 2014-01-23/27 xfabpag Unique campaign for installation and upgrade
# 2013-12-10 xfabpag Bug fixed in regex building for EXCLUDE_IF rule in the procWrapupAction section
# 2013-12-09 xfabpag Bug fixed (changed the regex used in the removeFromImm block)
# 2013-12-03 xfabpag Bug fixed (changed the regex used in the model changes blocK)
# 2013-11-27 xfabpag Bugs fixed in the support for both "-t" and "-f TEAM" options
#                    Added support for model changes for NO-AMF blocks (previously missing)
# 2013-11-20 xfabpag Bug fixed in procInitAction and procWrapupAction section generation (template template_end_type-mixed modified)
#                    Added support for model changes (added model_changes.sh in scripts/ folder, added template_model_changes in templates/ folder)
# 2013-11-19 xfabpag Bug fixed in Free Baseline Upgrade
#                    Bug fixed in CompBaseType section generation
#                    Regex changed in swRemove section for Free Baseline Upgrade (added template_sw_remove_fbu in templates/ folder)
#                    Regex changed in EXCLUDE_IF rule for the procWrapupAction section for Free Baseline Upgrade (template_wrapup_fbu_open modified)
# 2013-11-19 eanbuon Free Baseline Upgrade
# 2013-07-04 eattcla Dual AP handling 
# 2013-04-19 eoregua Modifies to allow upgrade also in case of different name of the bundle sdp from the base
#------------------------------------------------------------------------
# For example:
# ERIC-ACS_APSESH-CXC1371367-R1A01->ERIC-ACS_APSESHBIN-CXC1371367-R1A02
# ERIC-ACS_APBMBIN-CXC1371435-R1A05->ERIC-ACS_APBMBIN-CXC1371435-R1A06
# 
# In update.list:
#  APBM;R1A06;R1A05
#  APSESH;R1A02;R1A01;ERIC-ACS_APSESHBIN-CXC1371367-R1A02
#------------------------------------------------------------------------
#
# 2012-11-05 eanbuon Single Block Adaptation
# 2012-03-20 eanbuon First Release
##

#Loading Environment variables

PATH_UP_CONF="./conf"
BASE_BLOCK_FILE="$PATH_UP_CONF/blocks_base"
UP_CONF_NAME="UP.conf"
CC_CONF_FILE="$PATH_UP_CONF/cc_blocks_conf"
AP2_CAMP=$FALSE
AP2_FILE_NAME="AP2.conf"

. $PATH_UP_CONF/common_functions_UP

parse_cmdline $@

if [ "$OPT_TEAM" = $TRUE ] ; then
	# Define supervision variables
	SUPERVISION_SCRIPT_TIMEOUT=6000000000000
	SUPERVISION_BREAKPOINT_TIMEOUT=6000000000000

	#Define package info value
	UP_CAMPAIGN_NAME_1=ERIC-APG_$OPT_TEAM_BLOCK_NAME"_AP1"
	UP_CAMPAIGN_NAME_2=ERIC-APG_$OPT_TEAM_BLOCK_NAME"_AP2"
	UP_SUBTYPE_NAME_1=AP1
	UP_SUBTYPE_NAME_2=AP2
	UP_CAMPAIGN_FILE_NAME_1=$UP_CAMPAIGN_NAME_1.sdp
	UP_CAMPAIGN_FILE_NAME_2=$UP_CAMPAIGN_NAME_2.sdp
	UP_ACTIVATION_TIMEOUT=4444444
	UP_NAME=UP-$OPT_TEAM_BLOCK_NAME.tgz
	UP_PACKAGE_INFO_NAME=ERIC-$OPT_TEAM_BLOCK_NAME-INFO
	DOMAINID_NAME="APG43L"
	DOMAINID_PRODUCT_NUMBER=$(echo $OPT_TEAM_SW_VERSION|awk -F'-' '{print $1}')
	DOMAINID_PRODUCT_REVISION=$(echo $OPT_TEAM_SW_VERSION|awk -F'-' '{print $2}')
	DOMAINID_PRODUCT_DATE=$(date +"%m-%d-%Y")
	DOMAINID_DESCRIPTION="APG43L SW Version"
	DOMAINID_TYPE=""
	UP_PRODUCT_NAME="APG43L"
	UP_PRODUCT_NUMBER=$(echo $OPT_TEAM_SW_VERSION|awk -F'-' '{print $1}')
	UP_PRODUCT_REVISION=$(echo $OPT_TEAM_SW_VERSION|awk -F'-' '{print $2}')
	UP_PRODUCT_DATE=$(date +"%m-%d-%Y")
	UP_DESCRIPTION="APG43L SW Version"
	UP_ITEM=$(echo $OPT_TEAM_BLOCK_NAME | sed -e 's@[A-Z]*_@@g' -e 's@BIN$@@g')	
	UP_TYPE=""
else
	# Load configuration file for IO Developmtents creation of UP
	if [ -e $PATH_UP_CONF/$UP_CONF_NAME ]
	then
		. $PATH_UP_CONF/$UP_CONF_NAME
		DOMAINID_PRODUCT_DATE=$(date +"%m-%d-%Y")
		UP_PRODUCT_DATE=$(date +"%m-%d-%Y")
		UP_PACKAGE_INFO_NAME="APGUP"
		UP_CAMPAIGN_NAME_1="ERIC-APG_UP_AP1"
		UP_CAMPAIGN_NAME_2="ERIC-APG_UP_AP2"
		UP_DESCRIPTION="APG43L SW Version"
		UP_TYPE=""
		DOMAINID_NAME="$UP_PRODUCT_NAME"
		DOMAINID_PRODUCT_NUMBER="$UP_PRODUCT_NUMBER"
		DOMAINID_PRODUCT_REVISION="$UP_PRODUCT_REVISION"
		DOMAINID_DESCRIPTION="$UP_DESCRIPTION"
		DOMAINID_TYPE=""
		UP_SUBTYPE_NAME_1="AP1"
		UP_SUBTYPE_NAME_2="AP2"
		UP_CAMPAIGN_FILE_NAME_1="$UP_CAMPAIGN_NAME_1.sdp"
		UP_CAMPAIGN_FILE_NAME_2="$UP_CAMPAIGN_NAME_2.sdp"
		UP_ACTIVATION_TIMEOUT=4444444
		SUPERVISION_SCRIPT_TIMEOUT=6000000000000
		SUPERVISION_BREAKPOINT_TIMEOUT=6000000000000
	else 
		echo "Configuration Update file is missing!!!"
		exit 2
	fi		
fi
PATH_ENV_FILE="../IPA/conf"
PATH_SOURCE_FILE="../IPA/conf"
PATH_UPDATE_LIST="./conf"
PATH_OUTPUT_FOLDER="./out"
PATH_CAMPAIGN="./out"
PATH_PACKAGE_INFO="./out"
PATH_SDPS="./sdps"
PATH_SCRIPTS="./scripts"
BASELINE_FILE="acs_lct_sha1verify.conf"
BASELINE_SCRIPTS="baseline_selector.sh"
BASELINE_FILE_GEP12_AP1="AP1_GEP12_acs_lct_sha1verify.conf"
BASELINE_FILE_GEP12_AP2="AP2_GEP12_acs_lct_sha1verify.conf"
BASELINE_FILE_GEP5_AP1="AP1_GEP5_acs_lct_sha1verify.conf"
BASELINE_FILE_GEP5_AP2="AP2_GEP5_acs_lct_sha1verify.conf"
#Generating Temp folder
TMP_DIR=`mktemp -td swupdate_APG_XXX`
mkdir $TMP_DIR/out
TMP_DIR_OUT=$TMP_DIR/out
chmod 777 $TMP_DIR
chmod 777 $TMP_DIR_OUT
is_verbose && echo "INFO: temp path $TMP_DIR"
#check if the supervision period option is used and supervision script is available

is_verbose && echo "INFO: Supervision Period used"
if [ -f $PATH_SCRIPTS/supervision.sh ] ; then
	is_verbose && echo "INFO: Supervision script found"
else
	echo "ERROR: Supervision script NOT found"
	exit 2
fi
	
#clean output folder
if [ "$OPT_TEAM" = $TRUE ] ; then
	is_verbose && echo "INFO: No clean needed"
else
	rm -f $PATH_OUTPUT_FOLDER/*
	is_verbose && echo "INFO: Clean output folder"
fi
if [ "$OPT_RUP" = "TRUE" ] ; then
	readInfoFromRepositoryAll
elif [ "$OPT_LOCAL" = "TRUE" ] ; then
	readInfoFromRepositoryLocal
elif [ "$OPT_GROUP" = "TRUE" ] ; then
	readInfoFromRepositoryGroup
elif [ "$OPT_TEAM" = "TRUE" ] ; then
	is_verbose && echo "INFO: TEAM BFU - Initialization not needed"
fi

if [ "$OPT_TEAM" = "" ] ; then
	CHECK_SDPS=`cat $TMP_DIR/free.list | wc -l`
fi

#copy sdps under tmp folder
is_verbose && echo "INFO: Copy sdp under tmp folder..."
if [ "$OPT_TEAM" = $TRUE ] ; then
	if [ ! -d $OPT_TEAM_SDP_PATH ];then
		echo "ERROR Folder $OPT_TEAM_SDP_PATH not exists"
		exit 14
	fi
	cp $OPT_TEAM_SDP_PATH/*$OPT_TEAM_VERSION.x86_64.sdp $TMP_DIR_OUT/  &> /dev/null
	#clean not used sdps
	rm -f $TMP_DIR_OUT/*-I1-*
	rm -f $TMP_DIR_OUT/*-U1-*
	TEAM_SDP_FILE_NAME=$(ls -1 $TMP_DIR_OUT/| grep -Ei '*.sdp')
	if [ -z $TEAM_SDP_FILE_NAME ];then
		echo "ERROR Bundle SDP not found!"
		exit 14
	fi
else
	if [ "$OPT_LOCAL" = "TRUE" ] ; then
		cp $PATH_SDPS/* $TMP_DIR_OUT/
	elif [ "$OPT_TEAM" = "" ] ; then
		getBunldeFromRepository
	fi
fi
#Verify if exists and import env.conf file
#including configuration files
if [ -e $PATH_ENV_FILE/env.conf ]
then
	. $PATH_ENV_FILE/env.conf
else
	echo "Configuration file is missing!!!"
	exit 2
fi
#Verify if exists sources.list file

if [ -e $PATH_SOURCE_FILE/sources.list ]
then
	is_verbose && echo "INFO: sources.list detected"
else
	echo "sources.list file is missing!!!"
	exit 2
fi
#START CREATE CAMPAIGN
function makeCampaign () {
	UP_CAMPAIGN_NAME=$1
	
	if [ $UP_CAMPAIGN_NAME = $UP_CAMPAIGN_NAME_1 ] 
	then 
		UP_CAMPAIGN_FILE_NAME=$UP_CAMPAIGN_FILE_NAME_1
	else
		UP_CAMPAIGN_FILE_NAME=$UP_CAMPAIGN_FILE_NAME_2
	fi 
	
	is_verbose && echo "INFO: making $UP_CAMPAIGN_NAME campaign "
		
#check for the final file
	if [ -e $TMP_DIR/final.xml ]
	then
		rm $TMP_DIR/final.xml
		touch $TMP_DIR/final.xml
	else
		touch $TMP_DIR/final.xml
	fi

	if [ "$OPT_TEAM" = $TRUE ] ; then
		is_verbose && echo "INFO: Create update information"
		#Match amf information in sources.list using bundle name 
		while read line    
		do    
			BUNDLE_NAME=`echo $line | awk -F';' '{print $2}'`
			PRIM_NAME=`echo $BUNDLE_NAME | awk -F'-' '{print $2}'`
			if [ "$PRIM_NAME" = "$OPT_TEAM_BLOCK_NAME" ]
			then
				TEAM_HA_TYPE=`echo $line | awk -F';' '{print $1}'`
				TEAM_BUNDLE_NAME=`echo $line | awk -F';' '{print $2}'`
				TEAM_COMP_NAME=`echo $line | awk -F';' '{print $3}'`
				TEAM_COMP_PATH=`echo $line | awk -F';' '{print $4}'`
				if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ]
				then
					TEAM_COMP_TYPE_NAME=`echo $line | awk -F';' '{print $5}'`
					TEAM_CS_TYPE=`echo $line | awk -F';' '{print $6}'`
					TEAM_COMP_VERSION=`echo $line | awk -F';' '{print $7}'`
					TEAM_CLC_NAME=`echo $line | awk -F';' '{print $8}'`
					TEAM_HelthCheck=`echo $line | awk -F';' '{print $9}'`
					TEAM_SVC_TYPE=`echo $line | awk -F';' '{print $10}'`
					TEAM_CSI_NAME=`echo $line | awk -F';' '{print $11}'`
				fi
			fi
		done < $PATH_SOURCE_FILE/sources.list 
		TEAM_BUNDLE_BASE_NAME=`echo "$TEAM_BUNDLE_NAME"|sed 's/-[^-]*$//'`
		local bundle_base_name_wo_cxc=`echo "$TEAM_BUNDLE_BASE_NAME"|sed 's/-[^-]*$//'`
		TEAM_NEW_BUNDLE_NAME=`echo "$bundle_base_name_wo_cxc-$OPT_TEAM_CXC-$OPT_TEAM_VERSION"`
		if [ "$TEAM_HA_TYPE" = "2N" ]
		then
			IS_TWON_UPDATE=1
			IS_MIXED_UPDATE=1
		fi
		if [ "$TEAM_HA_TYPE" = "NORED" ]
		then
			IS_NORED_UPDATE=1
			IS_MIXED_UPDATE=1
		fi
		if [ "$TEAM_HA_TYPE" = "NOAMF" ]
		then
			IS_NOAMF_UPDATE=1
			IS_MIXED_UPDATE=0
		fi
	else
		#this check verify the type of UP:
		#	IS_MIXED_UPDATE=1 if the UP contains NOAMF && WITHAMF (NORED || 2N) COMPONENTS
		#	IS_MIXED_UPDATE=1 if the UP contains NORED || 2N components
		#	IS_MIXED_UPDATE=0 If the UP contains only NOAMF components
		IS_MIXED_UPDATE=1
		IS_NORED_UPDATE=0
		IS_TWON_UPDATE=0
		IS_NOAMF_UPDATE=0

		while read line_update
		do
			UPDATE_ITEM=`echo $line_update | awk -F';' '{print $1}'`
			UPDATE_ITEM_NAME=`echo $UPDATE_ITEM | awk -F'-' '{print $1}'`
			while read line    
			do    
				BUNDLE_NAME=`echo $line | awk -F';' '{print $2}'`
				PRIM_NAME=`echo $BUNDLE_NAME | awk -F'-' '{print $2}'`
				type=`echo $line | awk -F';' '{print $1}'`
				if [ "$UPDATE_ITEM_NAME" = "$PRIM_NAME" ]
				then
					if [ "$type" = "2N" ]
					then
						IS_TWON_UPDATE=1
					fi
					if [ "$type" = "NORED" ]
					then
						IS_NORED_UPDATE=1
					fi
					if [ "$type" = "NOAMF" ]
					then
						IS_NOAMF_UPDATE=1
					fi
				fi	
			done < $PATH_SOURCE_FILE/sources.list
		done < $TMP_DIR/free.list
		if [ "$IS_NOAMF_UPDATE" = "1" ]
		then
			if [ "$IS_NORED_UPDATE" = "0" ] && [ "$IS_TWON_UPDATE" = "0" ]
			then
				IS_MIXED_UPDATE=0
			fi
		fi

		# Create source list under tmp
		createSourceFileTmp $TMP_DIR
	fi
	#DEBUG
	#echo "DEBUG: IS_NORED_UPDATE: $IS_NORED_UPDATE"
	#echo "DEBUG: IS_TWON_UPDATE: $IS_TWON_UPDATE"
	#echo "DEBUG: IS_NOAMF_UPDATE: $IS_NOAMF_UPDATE"
	#echo "DEBUG: IS_MIXED_UPDATE: $IS_MIXED_UPDATE"

#START creating the intro part of the campaign ===================================================================================================================================================

	if [ "$IS_MIXED_UPDATE" = "1" ]
	then
		cp templates/template_initial-mixed $TMP_DIR/
		sed -i "s/#UP_CAMPAIGN_NAME#/$UP_CAMPAIGN_NAME/g" $TMP_DIR/template_initial-mixed
		cat $TMP_DIR/template_initial-mixed >> $TMP_DIR/final.xml
		rm -f $TMP_DIR/template_initial-mixed
	else
		cp templates/template_initial-1 $TMP_DIR/
		sed -i "s/#UP_CAMPAIGN_NAME#/$UP_CAMPAIGN_NAME/g" $TMP_DIR/template_initial-1
		cat $TMP_DIR/template_initial-1 >> $TMP_DIR/final.xml
		rm -f $TMP_DIR/template_initial-1
	fi
	is_verbose && echo "STEP: INTRO CREATION DONE"

#END creating the intro part of the campaign ===============================================================================================================================================


#START Creating component type name part for 2N and NoRed =====================================================================================================================================

	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ]
		then
			cp templates/template_comp_type-2 $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			sed -i "s/#COMP_VERSION#/$OPT_TEAM_VERSION/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			sed -i "s/#CS_TYPE#/$TEAM_CS_TYPE/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			sed -i "s/#CLC_NAME#/$TEAM_CLC_NAME/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			sed -i "s/#HCK#/$TEAM_HelthCheck/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			sed -i "s/#QUIESCING_COMPLETE_TIMEOUT#/$QUIESCING_COMPLETE_TIMEOUT/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			sed -i "s/#NEW_BUNDLE_NAME#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			if [ "$TEAM_COMP_TYPE_NAME" = "ERIC-APG_Agent" ]
			then
				sed -i "s/#RECOVERY_ON_ERROR#/$RECOVERY_ON_ERROR_AGENT/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
				sed -i "s/#CLC_CLI_TIMEOUT#/$CLC_CLI_TIMEOUT_AGENT/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
				sed -i "s/#CALLBACK_TIMEOUT#/$CALLBACK_TIMEOUT_AGENT/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
				sed -i "s/#HCK_MAX_DURATION#/$HCK_MAX_DURATION_AGENT/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
				sed -i "s/#HCK_PERIOD#/$HCK_PERIOD_AGENT/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			else
				sed -i "s/#RECOVERY_ON_ERROR#/$RECOVERY_ON_ERROR/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
				sed -i "s/#CLC_CLI_TIMEOUT#/$CLC_CLI_TIMEOUT/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
				sed -i "s/#CALLBACK_TIMEOUT#/$CALLBACK_TIMEOUT/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
				sed -i "s/#HCK_MAX_DURATION#/$HCK_MAX_DURATION/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
				sed -i "s/#HCK_PERIOD#/$HCK_PERIOD/g" $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
			fi
			cat $TMP_DIR/template_comp_type_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_comp_type_$TEAM_COMP_NAME
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ]
			then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
				COMP_VERSION=`echo $line_update | awk -F';' '{print $7}'`
				CLC_NAME=`echo $line_update | awk -F';' '{print $8}'`
				HelthCheck=`echo $line_update | awk -F';' '{print $9}'`
				CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`
				BUNDLE_BASE_NAME=`echo "$BUNDLE_NAME"|sed 's/-[^-]*$//'`

				NEW_BUNDLE_NAME=`echo "$BUNDLE_BASE_NAME-$COMP_VERSION"`

				cp templates/template_comp_type-2 $TMP_DIR/template_comp_type_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_comp_type_$COMP_NAME

				sed -i "s/#COMP_VERSION#/$COMP_VERSION/g" $TMP_DIR/template_comp_type_$COMP_NAME

				sed -i "s/#CS_TYPE#/$CS_TYPE/g" $TMP_DIR/template_comp_type_$COMP_NAME
				sed -i "s/#CLC_NAME#/$CLC_NAME/g" $TMP_DIR/template_comp_type_$COMP_NAME
				sed -i "s/#HCK#/$HelthCheck/g" $TMP_DIR/template_comp_type_$COMP_NAME
				sed -i "s/#NEW_BUNDLE_NAME#/$NEW_BUNDLE_NAME/g" $TMP_DIR/template_comp_type_$COMP_NAME
				sed -i "s/#QUIESCING_COMPLETE_TIMEOUT#/$QUIESCING_COMPLETE_TIMEOUT/g" $TMP_DIR/template_comp_type_$COMP_NAME
				if [ "$COMP_TYPE_NAME" = "ERIC-APG_Agent" ]
				then
					sed -i "s/#RECOVERY_ON_ERROR#/$RECOVERY_ON_ERROR_AGENT/g" $TMP_DIR/template_comp_type_$COMP_NAME
					sed -i "s/#CLC_CLI_TIMEOUT#/$CLC_CLI_TIMEOUT_AGENT/g" $TMP_DIR/template_comp_type_$COMP_NAME
					sed -i "s/#CALLBACK_TIMEOUT#/$CALLBACK_TIMEOUT_AGENT/g" $TMP_DIR/template_comp_type_$COMP_NAME
					sed -i "s/#HCK_MAX_DURATION#/$HCK_MAX_DURATION_AGENT/g" $TMP_DIR/template_comp_type_$COMP_NAME
					sed -i "s/#HCK_PERIOD#/$HCK_PERIOD_AGENT/g" $TMP_DIR/template_comp_type_$COMP_NAME
				else
					sed -i "s/#RECOVERY_ON_ERROR#/$RECOVERY_ON_ERROR/g" $TMP_DIR/template_comp_type_$COMP_NAME
					sed -i "s/#CLC_CLI_TIMEOUT#/$CLC_CLI_TIMEOUT/g" $TMP_DIR/template_comp_type_$COMP_NAME
					sed -i "s/#CALLBACK_TIMEOUT#/$CALLBACK_TIMEOUT/g" $TMP_DIR/template_comp_type_$COMP_NAME
					sed -i "s/#HCK_MAX_DURATION#/$HCK_MAX_DURATION/g" $TMP_DIR/template_comp_type_$COMP_NAME
					sed -i "s/#HCK_PERIOD#/$HCK_PERIOD/g" $TMP_DIR/template_comp_type_$COMP_NAME
				fi
				cat $TMP_DIR/template_comp_type_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_comp_type_$COMP_NAME
			fi
		done < $SOURCE_LIST_TMP 
	fi
	is_verbose && echo "STEP: COMP_TYPE CREATION DONE"
	
#END Creating component type name part for 2N and NoRed ===============================================================================================================

#START Creating SUBaseType for NoRed ============================================================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "NORED" ] ; then
			block_name=$(echo $TEAM_CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
			
			cp templates/template_sutype_fbu_open $TMP_DIR/template_sutype_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BLOCK_NAME#/$block_name/g" $TMP_DIR/template_sutype_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_sutype_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_sutype_fbu_open_$TEAM_COMP_NAME
			
			cp ../IPA/templates/TEMPLATE_SU_TYPE/template_su_type_nored_init $TMP_DIR/template_su_type_nored_init_$TEAM_COMP_NAME
			sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_su_type_nored_init_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_type_nored_init_$TEAM_COMP_NAME
			cat $TMP_DIR/template_su_type_nored_init_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_su_type_nored_init_$TEAM_COMP_NAME
			
			cp ../IPA/templates/TEMPLATE_SU_TYPE/template_su_type_component $TMP_DIR/template_su_type_component_$TEAM_COMP_NAME
			sed -i "s/#COMP_VERSION#/$OPT_TEAM_VERSION/g" $TMP_DIR/template_su_type_component_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_CS_TYPE/g" $TMP_DIR/template_su_type_component_$TEAM_COMP_NAME
			cat $TMP_DIR/template_su_type_component_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_su_type_component_$TEAM_COMP_NAME
			
			cp ../IPA/templates/TEMPLATE_SU_TYPE/template_svc_type_nored $TMP_DIR/template_svc_type_nored_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_svc_type_nored_$TEAM_COMP_NAME
			sed -i "s/#SAF_SVC_TYPE#/$block_name/g" $TMP_DIR/template_svc_type_nored_$TEAM_COMP_NAME
			cat $TMP_DIR/template_svc_type_nored_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_svc_type_nored_$TEAM_COMP_NAME
			
			cat ../IPA/templates/TEMPLATE_SU_TYPE/template_su_type_end >> $TMP_DIR/final.xml
			
			cat templates/template_sutype_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update ; do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "NORED" ] ; then
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
				block_name=$(echo $CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
				
				cp templates/template_sutype_fbu_open $TMP_DIR/template_sutype_fbu_open_$COMP_NAME
				sed -i "s/#BLOCK_NAME#/$block_name/g" $TMP_DIR/template_sutype_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_sutype_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_sutype_fbu_open_$COMP_NAME
				
				cp ../IPA/templates/TEMPLATE_SU_TYPE/template_su_type_nored_init $TMP_DIR/template_su_type_nored_init_$COMP_NAME
				sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_su_type_nored_init_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_type_nored_init_$COMP_NAME
				cat $TMP_DIR/template_su_type_nored_init_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_su_type_nored_init_$COMP_NAME
				
				cp ../IPA/templates/TEMPLATE_SU_TYPE/template_su_type_component $TMP_DIR/template_su_type_component_$COMP_NAME
				sed -i "s/#COMP_VERSION#/$NEW_RSTATE/g" $TMP_DIR/template_su_type_component_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$CS_TYPE/g" $TMP_DIR/template_su_type_component_$COMP_NAME
				cat $TMP_DIR/template_su_type_component_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_su_type_component_$COMP_NAME
				
				cp ../IPA/templates/TEMPLATE_SU_TYPE/template_svc_type_nored $TMP_DIR/template_svc_type_nored_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_svc_type_nored_$COMP_NAME
				sed -i "s/#SAF_SVC_TYPE#/$block_name/g" $TMP_DIR/template_svc_type_nored_$COMP_NAME
				cat $TMP_DIR/template_svc_type_nored_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_svc_type_nored_$COMP_NAME
				
				cat ../IPA/templates/TEMPLATE_SU_TYPE/template_su_type_end >> $TMP_DIR/final.xml
				
				cat templates/template_sutype_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SU_TYPE CREATION DONE"
#END Creating SUBaseType for NoRed ==============================================================================================================================================================================

#START Creating SGBaseType for NoRed ============================================================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "NORED" ] ; then
			block_name=$(echo $TEAM_CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
			
			cp templates/template_sgtype_fbu_open $TMP_DIR/template_sgtype_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BLOCK_NAME#/$block_name/g" $TMP_DIR/template_sgtype_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_sgtype_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_sgtype_fbu_open_$TEAM_COMP_NAME
			
			cp ../IPA/templates/TEMPLATE_SG_TYPE/template_SG_TYPE_NORED $TMP_DIR/template_SG_TYPE_NORED_$TEAM_COMP_NAME
			sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_SG_TYPE_NORED_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_SG_TYPE_NORED_$TEAM_COMP_NAME
			sed -i "s/#COMP_RESTART_MAX#/$COMP_RESTART_MAX_NORED/g" $TMP_DIR/template_SG_TYPE_NORED_$TEAM_COMP_NAME
			sed -i "s/#COMP_RESTART_PROB#/$COMP_RESTART_PROB_NORED/g" $TMP_DIR/template_SG_TYPE_NORED_$TEAM_COMP_NAME
			sed -i "s/#SU_REST_PROB#/$SU_REST_PROB_NORED/g" $TMP_DIR/template_SG_TYPE_NORED_$TEAM_COMP_NAME
			cat $TMP_DIR/template_SG_TYPE_NORED_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_SG_TYPE_NORED_$TEAM_COMP_NAME
			
			cat templates/template_sgtype_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update ; do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "NORED" ] ; then
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
				block_name=$(echo $CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
				
				cp templates/template_sgtype_fbu_open $TMP_DIR/template_sgtype_fbu_open_$COMP_NAME
				sed -i "s/#BLOCK_NAME#/$block_name/g" $TMP_DIR/template_sgtype_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_sgtype_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_sgtype_fbu_open_$COMP_NAME
				
				cp ../IPA/templates/TEMPLATE_SG_TYPE/template_SG_TYPE_NORED $TMP_DIR/template_SG_TYPE_NORED_$COMP_NAME
				sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_SG_TYPE_NORED_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_SG_TYPE_NORED_$COMP_NAME
				sed -i "s/#COMP_RESTART_MAX#/$COMP_RESTART_MAX_NORED/g" $TMP_DIR/template_SG_TYPE_NORED_$COMP_NAME
				sed -i "s/#COMP_RESTART_PROB#/$COMP_RESTART_PROB_NORED/g" $TMP_DIR/template_SG_TYPE_NORED_$COMP_NAME
				sed -i "s/#SU_REST_PROB#/$SU_REST_PROB_NORED/g" $TMP_DIR/template_SG_TYPE_NORED_$COMP_NAME
				cat $TMP_DIR/template_SG_TYPE_NORED_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_SG_TYPE_NORED_$COMP_NAME
				
				cat templates/template_sgtype_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SG_TYPE CREATION DONE"
#END Creating SGBaseType for NoRed ==============================================================================================================================================================================

#START Creating CS Type for 2N and NoRed ============================================================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ]
		then
			cp templates/template_cstype_fbu_open $TMP_DIR/template_cstype_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#CS_TYPE#/$TEAM_CS_TYPE/g" $TMP_DIR/template_cstype_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_cstype_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_cstype_fbu_open_$TEAM_COMP_NAME
			
			cp ../IPA/templates/TEMPLATE_CS_TYPE/template_CS_type $TMP_DIR/template_CS_type_$TEAM_COMP_NAME
			sed -i "s/#CS_TYPE#/$TEAM_CS_TYPE/g" $TMP_DIR/template_CS_type_$TEAM_COMP_NAME
			cat $TMP_DIR/template_CS_type_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_CS_type_$TEAM_COMP_NAME

			cat templates/template_cstype_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ]
			then
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
				
				cp templates/template_cstype_fbu_open $TMP_DIR/template_cstype_fbu_open_$COMP_NAME
				sed -i "s/#CS_TYPE#/$CS_TYPE/g" $TMP_DIR/template_cstype_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_cstype_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_cstype_fbu_open_$COMP_NAME
				
				cp ../IPA/templates/TEMPLATE_CS_TYPE/template_CS_type $TMP_DIR/template_CS_type_$COMP_NAME
				sed -i "s/#CS_TYPE#/$CS_TYPE/g" $TMP_DIR/template_CS_type_$COMP_NAME
				cat $TMP_DIR/template_CS_type_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_CS_type_$COMP_NAME

				cat templates/template_cstype_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP 
	fi
	is_verbose && echo "STEP: CS_TYPE CREATION DONE"
#END Creating CS Type for 2N and NoRed ==============================================================================================================================================================================

#START Creating ServiceBaseType ============================================================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ]
		then
			cp templates/template_svctype_fbu_open $TMP_DIR/template_svctype_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/ERIC-$TEAM_SVC_TYPE/g" $TMP_DIR/template_svctype_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_svctype_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_svctype_fbu_open_$TEAM_COMP_NAME
			
			if [ "$TEAM_HA_TYPE" = "2N" ] ; then
				cp ../IPA/templates/TEMPLATE_SERVICE_BASE_TYPE/template_service_base_type_init $TMP_DIR/template_service_base_type_init_$TEAM_COMP_NAME
			else
				cp ../IPA/templates/TEMPLATE_SERVICE_BASE_TYPE/template_service_base_type_init_nored $TMP_DIR/template_service_base_type_init_$TEAM_COMP_NAME
			fi
			sed -i "s/#SAF_SVC_TYPE#/$TEAM_SVC_TYPE/g" $TMP_DIR/template_service_base_type_init_$TEAM_COMP_NAME
			sed -i "s/#CSI_NAME#/$TEAM_CSI_NAME/g" $TMP_DIR/template_service_base_type_init_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_service_base_type_init_$TEAM_COMP_NAME
			cat $TMP_DIR/template_service_base_type_init_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_service_base_type_init_$TEAM_COMP_NAME
			
			cp ../IPA/templates/TEMPLATE_SERVICE_BASE_TYPE/template_service_base_type_cs $TMP_DIR/template_service_base_type_cs_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_service_base_type_cs_$TEAM_COMP_NAME
			sed -i "s/#CS_TYPE#/$TEAM_CS_TYPE/g" $TMP_DIR/template_service_base_type_cs_$TEAM_COMP_NAME
			cat $TMP_DIR/template_service_base_type_cs_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_service_base_type_cs_$TEAM_COMP_NAME
			
			cat ../IPA/templates/TEMPLATE_SERVICE_BASE_TYPE/template_service_base_type_end >> $TMP_DIR/final.xml

			cat templates/template_svctype_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ]
			then
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
				SVC_TYPE=`echo $line_update | awk -F';' '{print $10}'`
				CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`

				cp templates/template_svctype_fbu_open $TMP_DIR/template_svctype_fbu_open_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/ERIC-$SVC_TYPE/g" $TMP_DIR/template_svctype_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_svctype_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_svctype_fbu_open_$COMP_NAME
				
				if [ "$type" = "2N" ] ; then
					cp ../IPA/templates/TEMPLATE_SERVICE_BASE_TYPE/template_service_base_type_init $TMP_DIR/template_service_base_type_init_$COMP_NAME
				else
					cp ../IPA/templates/TEMPLATE_SERVICE_BASE_TYPE/template_service_base_type_init_nored $TMP_DIR/template_service_base_type_init_$COMP_NAME
				fi
				sed -i "s/#SAF_SVC_TYPE#/$SVC_TYPE/g" $TMP_DIR/template_service_base_type_init_$COMP_NAME
				sed -i "s/#CSI_NAME#/$CSI_NAME/g" $TMP_DIR/template_service_base_type_init_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_service_base_type_init_$COMP_NAME
				cat $TMP_DIR/template_service_base_type_init_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_service_base_type_init_$COMP_NAME
				
				cp ../IPA/templates/TEMPLATE_SERVICE_BASE_TYPE/template_service_base_type_cs $TMP_DIR/template_service_base_type_cs_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_service_base_type_cs_$COMP_NAME
				sed -i "s/#CS_TYPE#/$CS_TYPE/g" $TMP_DIR/template_service_base_type_cs_$COMP_NAME
				cat $TMP_DIR/template_service_base_type_cs_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_service_base_type_cs_$COMP_NAME
				
				cat ../IPA/templates/TEMPLATE_SERVICE_BASE_TYPE/template_service_base_type_end >> $TMP_DIR/final.xml

				cat templates/template_svctype_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP 
	fi
	is_verbose && echo "STEP: SERVICEBASETYPE CREATION DONE"
#END Creating ServiceBaseType ==============================================================================================================================================================================

	if [ "$IS_MIXED_UPDATE" = "1" ]
	then
		cat templates/template_end_type-mixed >> $TMP_DIR/final.xml
	else
		cat templates/template_end_type-3 >> $TMP_DIR/final.xml
	fi

#START Creating MODEL_CHANGES part =====================================================================================================================================
	if [ "$OPT_TEAM" = "" ] ; then
		while read line_update ; do
			type=`echo $line_update | awk -F';' '{print $1}'`
			BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
			COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
			COMP_VERSION=`echo $line_update | awk -F';' '{print $7}'`
			BUNDLE_BASE_NAME=`echo "$BUNDLE_NAME"|sed 's/-[^-]*$//'`

			block_name=$(echo $BUNDLE_NAME | awk -F'-' '{ print $2 }')
			versions_to_install="$(installationTagCheck $block_name)"
			if [ "$versions_to_install" != "" ] ; then
				cp templates/template_procinit_modelch_fbu_open $TMP_DIR/template_procinit_modelch_fbu_open_$COMP_NAME
				sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_procinit_modelch_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_modelch_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_modelch_fbu_open_$COMP_NAME
			fi

			if [ "$type" = "2N" ] || [ "$type" = "NORED" ] ; then
				NEW_BUNDLE_NAME=`echo "$BUNDLE_BASE_NAME-$COMP_VERSION"`
			else
				NEW_BUNDLE_NAME=$BUNDLE_NAME
			fi
			cp templates/template_model_changes $TMP_DIR/template_model_changes_$COMP_NAME
			sed -i "s/#NEW_BUNDLE_NAME#/$NEW_BUNDLE_NAME/g" $TMP_DIR/template_model_changes_$COMP_NAME
			new_bundle_base_name=`echo "$NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			sed -i "s/#NEW_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_model_changes_$COMP_NAME
			cat $TMP_DIR/template_model_changes_$COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_model_changes_$COMP_NAME
			if [ "$versions_to_install" != "" ] ; then
				cat templates/template_procinit_modelch_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	else
		block_name=$(echo $TEAM_NEW_BUNDLE_NAME | awk -F'-' '{ print $2 }')
		versions_to_install="$(installationTagCheck $block_name)"
		if [ "$versions_to_install" != "" ] ; then
			cp templates/template_procinit_modelch_fbu_open $TMP_DIR/template_procinit_modelch_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_procinit_modelch_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_modelch_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_modelch_fbu_open_$TEAM_COMP_NAME
		fi

		cp templates/template_model_changes $TMP_DIR/template_model_changes_$TEAM_COMP_NAME
		sed -i "s/#NEW_BUNDLE_NAME#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_model_changes_$TEAM_COMP_NAME
		new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
		sed -i "s/#NEW_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_model_changes_$TEAM_COMP_NAME

		## do not do this for maiden installation
		##cat $TMP_DIR/template_model_changes_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
		
		rm -f $TMP_DIR/template_model_changes_$TEAM_COMP_NAME
		if [ "$versions_to_install" != "" ] ; then
			cat templates/template_procinit_modelch_fbu_end >> $TMP_DIR/final.xml
		fi
	fi
	is_verbose && echo "STEP: MODEL_CHANGES CREATION DONE"
#END Creating MODEL_CHANGES part =====================================================================================================================================

#START Creating SU COMP TYPE part =====================================================================================================================================

	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ]
		then		
			cp templates/template_su_comp_type-4 $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#NEW_RSTATE#/$OPT_TEAM_VERSION/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#SU_TYPE#/$SU_2N_TYPE/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME

			NEW_RSTATE=`echo $TEAM_NEW_BUNDLE_NAME | awk -F'-' '{print $4}'`
			cp templates/template_procinit_fbu_open $TMP_DIR/template_procinit_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_procinit_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#NEW_RSTATE#/$NEW_RSTATE/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#SU_TYPE#/$SU_2N_TYPE/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml

			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			cat $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME >> $TMP_DIR/final.xml					
			cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

			cat templates/template_procinit_fbu_end >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_fbu_open_$TEAM_COMP_NAME

			rm -f $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
		fi
		if [ "$TEAM_HA_TYPE" = "NORED" ]
		then	
			cp templates/template_su_comp_type-4 $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#NEW_RSTATE#/$OPT_TEAM_VERSION/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#SU_TYPE#/ERIC-APG_SU_$TEAM_CSI_NAME/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME

			NEW_RSTATE=`echo $TEAM_NEW_BUNDLE_NAME | awk -F'-' '{print $4}'`
			cp templates/template_procinit_fbu_open $TMP_DIR/template_procinit_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_procinit_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#NEW_RSTATE#/$NEW_RSTATE/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#SU_TYPE#/ERIC-APG_SU_$TEAM_CSI_NAME/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml

			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			cat $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			cat templates/template_procinit_imm_end	 >> $TMP_DIR/final.xml

			cat templates/template_procinit_fbu_end >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_fbu_open_$TEAM_COMP_NAME

			rm -f $TMP_DIR/template_su_comp_type_$TEAM_COMP_NAME
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			# 2N
			if [ "$type" = "2N" ]
			then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`
				cp templates/template_su_comp_type-4 $TMP_DIR/template_su_comp_type_$COMP_NAME
				cp templates/template_procinit_fbu_open $TMP_DIR/template_procinit_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE#/$BUNDLE_NAME/g" $TMP_DIR/template_procinit_fbu_open_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_su_comp_type_$COMP_NAME
				sed -i "s/#NEW_RSTATE#/$NEW_RSTATE/g" $TMP_DIR/template_su_comp_type_$COMP_NAME
				sed -i "s/#SU_TYPE#/$SU_2N_TYPE/g" $TMP_DIR/template_su_comp_type_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_comp_type_$COMP_NAME
				cat $TMP_DIR/template_procinit_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				cat $TMP_DIR/template_su_comp_type_$COMP_NAME >> $TMP_DIR/final.xml	
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml
				cat templates/template_procinit_fbu_end >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_fbu_open_$COMP_NAME
				rm -f $TMP_DIR/template_su_comp_type_$COMP_NAME
			fi
			# NORED
			if [ "$type" = "NORED" ]
			then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`
				NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
				cp templates/template_su_comp_type-4 $TMP_DIR/template_su_comp_type_$COMP_NAME
				cp templates/template_procinit_fbu_open $TMP_DIR/template_procinit_fbu_open_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_su_comp_type_$COMP_NAME
				sed -i "s/#BFU_BUNDLE#/$BUNDLE_NAME/g" $TMP_DIR/template_procinit_fbu_open_$COMP_NAME
				sed -i "s/#NEW_RSTATE#/$NEW_RSTATE/g" $TMP_DIR/template_su_comp_type_$COMP_NAME
				sed -i "s/#SU_TYPE#/ERIC-APG_SU_$CSI_NAME/g" $TMP_DIR/template_su_comp_type_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_comp_type_$COMP_NAME
				cat $TMP_DIR/template_procinit_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				cat $TMP_DIR/template_su_comp_type_$COMP_NAME >> $TMP_DIR/final.xml
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml
				cat templates/template_procinit_fbu_end >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_fbu_open_$COMP_NAME
				rm -f $TMP_DIR/template_su_comp_type_$COMP_NAME
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SU COMP TYPE CREATION DONE"

#END Creating SU COMP TYPE part ==============================================================================================================================================

#START Creating APPS_PROCINIT part =====================================================================================================================================
	if [ "$OPT_TEAM" = "" ] ; then
		while read line_update ; do
			type=`echo $line_update | awk -F';' '{print $1}'`
			BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
			COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
			COMP_VERSION=`echo $line_update | awk -F';' '{print $7}'`
			BUNDLE_BASE_NAME=`echo "$BUNDLE_NAME"|sed 's/-[^-]*$//'`
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ] ; then
				NEW_BUNDLE_NAME=`echo "$BUNDLE_BASE_NAME-$COMP_VERSION"`
			else
				NEW_BUNDLE_NAME=$BUNDLE_NAME
			fi
			new_bundle_base_name=`echo "$NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
			
			cp templates/template_apps_procinit_fbu $TMP_DIR/template_apps_procinit_$COMP_NAME
			sed -i "s/#BUNDLE_NAME#/$NEW_BUNDLE_NAME/g" $TMP_DIR/template_apps_procinit_$COMP_NAME
			cat $TMP_DIR/template_apps_procinit_$COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_apps_procinit_$COMP_NAME
			
			cat templates/template_apps_end_fbu >> $TMP_DIR/final.xml
			
			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
		done < $SOURCE_LIST_TMP
	else
		new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
		cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
		sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
		cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
		rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
		
		cp templates/template_apps_procinit_fbu $TMP_DIR/template_apps_procinit_$TEAM_COMP_NAME
		sed -i "s/#BUNDLE_NAME#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_apps_procinit_$TEAM_COMP_NAME
		cat $TMP_DIR/template_apps_procinit_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
		rm -f $TMP_DIR/template_apps_procinit_$TEAM_COMP_NAME
		
		cat templates/template_apps_end_fbu >> $TMP_DIR/final.xml
		
		cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
	fi
	is_verbose && echo "STEP: APPS_PROCINIT CREATION DONE"
#END Creating APPS_PROCINIT part =====================================================================================================================================

#START Creating SA AMF SG part for NORED =====================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "NORED" ] ; then
			block_name=$(echo $TEAM_CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			
			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			
			cp ../IPA/templates/TEMPLATE_AP_APPS/template_sg_nored $TMP_DIR/template_sg_nored_$TEAM_COMP_NAME
			sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_sg_nored_$TEAM_COMP_NAME
			sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_sg_nored_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_sg_nored_$TEAM_COMP_NAME
			cat $TMP_DIR/template_sg_nored_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_sg_nored_$TEAM_COMP_NAME
			
			cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml
			
			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update ; do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "NORED" ] ; then
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
				block_name=$(echo $CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				
				cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				
				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				
				cp ../IPA/templates/TEMPLATE_AP_APPS/template_sg_nored $TMP_DIR/template_sg_nored_$COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_sg_nored_$COMP_NAME
				sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_sg_nored_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_sg_nored_$COMP_NAME
				cat $TMP_DIR/template_sg_nored_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_sg_nored_$COMP_NAME
				
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml
				
				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SA AMF SG CREATION DONE"
#END Creating SA AMF SG part for NORED =======================================================================================================================================

#START Creating SA AMF SU part for NORED =====================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "NORED" ] ; then
			block_name=$(echo $TEAM_CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			
			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			
			cp ../IPA/templates/TEMPLATE_AP_APPS/template_su_nored_1 $TMP_DIR/template_su_nored_1_$TEAM_COMP_NAME
			sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_su_nored_1_$TEAM_COMP_NAME
			sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_su_nored_1_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_nored_1_$TEAM_COMP_NAME
			cat $TMP_DIR/template_su_nored_1_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_su_nored_1_$TEAM_COMP_NAME
			
			cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml
				
			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			
			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			
			cp ../IPA/templates/TEMPLATE_AP_APPS/template_su_nored_2 $TMP_DIR/template_su_nored_2_$TEAM_COMP_NAME
			sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_su_nored_2_$TEAM_COMP_NAME
			sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_su_nored_2_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_nored_2_$TEAM_COMP_NAME
			cat $TMP_DIR/template_su_nored_2_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_su_nored_2_$TEAM_COMP_NAME
			
			cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml
				
			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update ; do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "NORED" ] ; then
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
				block_name=$(echo $CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				
				cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				
				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				
				cp ../IPA/templates/TEMPLATE_AP_APPS/template_su_nored_1 $TMP_DIR/template_su_nored_1_$COMP_NAME
				sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_su_nored_1_$COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_su_nored_1_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_nored_1_$COMP_NAME
				cat $TMP_DIR/template_su_nored_1_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_su_nored_1_$COMP_NAME
				
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml
					
				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
				
				cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				
				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				
				cp ../IPA/templates/TEMPLATE_AP_APPS/template_su_nored_2 $TMP_DIR/template_su_nored_2_$COMP_NAME
				sed -i "s/#CSI_NAME#/$block_name/g" $TMP_DIR/template_su_nored_2_$COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_su_nored_2_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_su_nored_2_$COMP_NAME
				cat $TMP_DIR/template_su_nored_2_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_su_nored_2_$COMP_NAME
				
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml
					
				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SA AMF SU CREATION DONE"
#END Creating SA AMF SU part for NORED =======================================================================================================================================

#START Creating SA AMF COMP part for 2N and NORED =====================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ] ; then

			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			
			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			if [ "$TEAM_HA_TYPE" = "2N" ] ; then
				cp ../IPA/templates/TEMPLATE_COMP_2N/TEMPLATE_COMP_2N_1 $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			else
				cp ../IPA/templates/TEMPLATE_COMP_NO_RED/TEMPLATE_COMP_NO_RED1 $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			fi
			sed -i "s/#SU1_2N#/1/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#SG_2N#/2N/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#COMP_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#COMP_VERSION#/$OPT_TEAM_VERSION/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#HCK#/$TEAM_HelthCheck/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#HCK_PERIOD_2N#/$HCK_PERIOD/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#HCK_MAX_DURATION_2N#/$HCK_MAX_DURATION/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#CS_TYPE#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			
			sed -i "s/#CSI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#HCK_PERIOD#/$HCK_PERIOD/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			sed -i "s/#HCK_MAX_DURATION#/$HCK_MAX_DURATION/g" $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			cat $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/TEMPLATE_COMP1_$TEAM_COMP_NAME
			cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			
			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME

			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			if [ "$TEAM_HA_TYPE" = "2N" ] ; then
				cp ../IPA/templates/TEMPLATE_COMP_2N/TEMPLATE_COMP_2N_2 $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			else
				cp ../IPA/templates/TEMPLATE_COMP_NO_RED/TEMPLATE_COMP_NO_RED2 $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			fi
			sed -i "s/#SU2_2N#/2/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#SG_2N#/2N/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#COMP_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#COMP_VERSION#/$OPT_TEAM_VERSION/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#HCK#/$TEAM_HelthCheck/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#HCK_PERIOD_2N#/$HCK_PERIOD/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#HCK_MAX_DURATION_2N#/$HCK_MAX_DURATION/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#CS_TYPE#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			
			sed -i "s/#CSI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#HCK_PERIOD#/$HCK_PERIOD/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			sed -i "s/#HCK_MAX_DURATION#/$HCK_MAX_DURATION/g" $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			cat $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/TEMPLATE_COMP2_$TEAM_COMP_NAME
			cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ] ; then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`
				HelthCheck=`echo $line_update | awk -F';' '{print $9}'`
				
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
					
				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				if [ "$type" = "2N" ] ; then
					cp ../IPA/templates/TEMPLATE_COMP_2N/TEMPLATE_COMP_2N_1 $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				else
					cp ../IPA/templates/TEMPLATE_COMP_NO_RED/TEMPLATE_COMP_NO_RED1 $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				fi
				sed -i "s/#SU1_2N#/1/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#SG_2N#/2N/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#COMP_NAME#/$COMP_NAME/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#COMP_VERSION#/$NEW_RSTATE/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#HCK#/$HelthCheck/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#HCK_PERIOD_2N#/$HCK_PERIOD/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#HCK_MAX_DURATION_2N#/$HCK_MAX_DURATION/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#CS_TYPE#/$COMP_TYPE_NAME/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				
				sed -i "s/#CSI_NAME#/$COMP_NAME/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#HCK_PERIOD#/$HCK_PERIOD/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				sed -i "s/#HCK_MAX_DURATION#/$HCK_MAX_DURATION/g" $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				cat $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/TEMPLATE_COMP1_$COMP_NAME
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
				
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME

				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				if [ "$type" = "2N" ] ; then
					cp ../IPA/templates/TEMPLATE_COMP_2N/TEMPLATE_COMP_2N_2 $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				else
					cp ../IPA/templates/TEMPLATE_COMP_NO_RED/TEMPLATE_COMP_NO_RED2 $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				fi
				sed -i "s/#SU2_2N#/2/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#SG_2N#/2N/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#COMP_NAME#/$COMP_NAME/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#COMP_VERSION#/$NEW_RSTATE/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#HCK#/$HelthCheck/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#HCK_PERIOD_2N#/$HCK_PERIOD/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#HCK_MAX_DURATION_2N#/$HCK_MAX_DURATION/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#CS_TYPE#/$COMP_TYPE_NAME/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				
				sed -i "s/#CSI_NAME#/$COMP_NAME/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#HCK_PERIOD#/$HCK_PERIOD/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				sed -i "s/#HCK_MAX_DURATION#/$HCK_MAX_DURATION/g" $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				cat $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/TEMPLATE_COMP2_$COMP_NAME
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SA AMF COMP CREATION DONE"
#END Creating SA AMF COMP part for 2N and NORED =====================================================================================================================================

#START Creating SA AMF SI part for 2N and NORED =====================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ] ; then
			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME

			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			if [ "$TEAM_HA_TYPE" = "2N" ] ; then
				cp templates/template_si_fbu_2n $TMP_DIR/template_si_2n_$TEAM_COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_si_2n_$TEAM_COMP_NAME
				sed -i "s/#SAF_SI#/$TEAM_COMP_NAME/g" $TMP_DIR/template_si_2n_$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_si_2n_$TEAM_COMP_NAME
				sed -i "s/#SAF_SVC_TYPE#/$TEAM_SVC_TYPE/g" $TMP_DIR/template_si_2n_$TEAM_COMP_NAME
				sed -i "s/#SG_2N#/2N/g" $TMP_DIR/template_si_2n_$TEAM_COMP_NAME
				sed -i "s/#SI_RANK_2N#/2/g" $TMP_DIR/template_si_2n_$TEAM_COMP_NAME
				cat $TMP_DIR/template_si_2n_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_si_2n_$TEAM_COMP_NAME
				
				cp ../IPA/templates/TEMPLATE_SI/template_si_dependency $TMP_DIR/template_si_dependency_$TEAM_COMP_NAME
				sed -i "s/#SAF_SVC_TYPE#/$TEAM_COMP_NAME/g" $TMP_DIR/template_si_dependency_$TEAM_COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_si_dependency_$TEAM_COMP_NAME
				sed -i "s/#SI_NAME_AGENT#/AGENT/g" $TMP_DIR/template_si_dependency_$TEAM_COMP_NAME
				cat $TMP_DIR/template_si_dependency_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_si_dependency_$TEAM_COMP_NAME
			else
				cp templates/template_si_fbu_nored_1 $TMP_DIR/template_si_nored_1_$TEAM_COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_si_nored_1_$TEAM_COMP_NAME
				sed -i "s/#CSI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/template_si_nored_1_$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_si_nored_1_$TEAM_COMP_NAME
				sed -i "s/#SI_RANK_NORED#/1/g" $TMP_DIR/template_si_nored_1_$TEAM_COMP_NAME
				cat $TMP_DIR/template_si_nored_1_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_si_nored_1_$TEAM_COMP_NAME
				
				cp templates/template_si_fbu_nored_2 $TMP_DIR/template_si_nored_2_$TEAM_COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_si_nored_2_$TEAM_COMP_NAME
				sed -i "s/#CSI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/template_si_nored_2_$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_si_nored_2_$TEAM_COMP_NAME
				sed -i "s/#SI_RANK_NORED#/1/g" $TMP_DIR/template_si_nored_2_$TEAM_COMP_NAME
				cat $TMP_DIR/template_si_nored_2_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_si_nored_2_$TEAM_COMP_NAME
			fi
			cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ] ; then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				SVC_TYPE=`echo $line_update | awk -F';' '{print $10}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`
				HelthCheck=`echo $line_update | awk -F';' '{print $9}'`
				
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME

				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				if [ "$type" = "2N" ] ; then
					cp templates/template_si_fbu_2n $TMP_DIR/template_si_2n_$COMP_NAME
					sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_si_2n_$COMP_NAME
					sed -i "s/#SAF_SI#/$COMP_NAME/g" $TMP_DIR/template_si_2n_$COMP_NAME
					sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_si_2n_$COMP_NAME
					sed -i "s/#SAF_SVC_TYPE#/$SVC_TYPE/g" $TMP_DIR/template_si_2n_$COMP_NAME
					sed -i "s/#SG_2N#/2N/g" $TMP_DIR/template_si_2n_$COMP_NAME
					sed -i "s/#SI_RANK_2N#/2/g" $TMP_DIR/template_si_2n_$COMP_NAME
					cat $TMP_DIR/template_si_2n_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_si_2n_$COMP_NAME
					
					cp ../IPA/templates/TEMPLATE_SI/template_si_dependency $TMP_DIR/template_si_dependency_$COMP_NAME
					sed -i "s/#SAF_SVC_TYPE#/$COMP_NAME/g" $TMP_DIR/template_si_dependency_$COMP_NAME
					sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_si_dependency_$COMP_NAME
					sed -i "s/#SI_NAME_AGENT#/AGENT/g" $TMP_DIR/template_si_dependency_$COMP_NAME
					cat $TMP_DIR/template_si_dependency_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_si_dependency_$COMP_NAME
				else
					cp templates/template_si_fbu_nored_1 $TMP_DIR/template_si_nored_1_$COMP_NAME
					sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_si_nored_1_$COMP_NAME
					sed -i "s/#CSI_NAME#/$COMP_NAME/g" $TMP_DIR/template_si_nored_1_$COMP_NAME
					sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_si_nored_1_$COMP_NAME
					sed -i "s/#SI_RANK_NORED#/1/g" $TMP_DIR/template_si_nored_1_$COMP_NAME
					cat $TMP_DIR/template_si_nored_1_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_si_nored_1_$COMP_NAME
					
					cp templates/template_si_fbu_nored_2 $TMP_DIR/template_si_nored_2_$COMP_NAME
					sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_si_nored_2_$COMP_NAME
					sed -i "s/#CSI_NAME#/$COMP_NAME/g" $TMP_DIR/template_si_nored_2_$COMP_NAME
					sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_si_nored_2_$COMP_NAME
					sed -i "s/#SI_RANK_NORED#/1/g" $TMP_DIR/template_si_nored_2_$COMP_NAME
					cat $TMP_DIR/template_si_nored_2_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_si_nored_2_$COMP_NAME
				fi
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SA AMF SI CREATION DONE"
#END Creating SA AMF SI part for 2N and NORED =====================================================================================================================================

#START Creating SA AMF CSI part for 2N and NORED =====================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ] ; then

			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME

			cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
			if [ "$TEAM_HA_TYPE" = "2N" ] ; then
				cp ../IPA/templates/TEMPLATE_CSI/template_csi_2n $TMP_DIR/template_csi_2n_$TEAM_COMP_NAME
				sed -i "s/#SI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/template_csi_2n_$TEAM_COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_csi_2n_$TEAM_COMP_NAME
				sed -i "s/#CSI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/template_csi_2n_$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_csi_2n_$TEAM_COMP_NAME
				sed -i "s/#CS_TYPE#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_csi_2n_$TEAM_COMP_NAME
				cat $TMP_DIR/template_csi_2n_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_csi_2n_$TEAM_COMP_NAME
			else
				cp ../IPA/templates/TEMPLATE_CSI/template_csi_nored_1 $TMP_DIR/template_csi_nored_1_$TEAM_COMP_NAME
				sed -i "s/#CSI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/template_csi_nored_1_$TEAM_COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_csi_nored_1_$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_csi_nored_1_$TEAM_COMP_NAME
				sed -i "s/#CS_TYPE#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_csi_nored_1_$TEAM_COMP_NAME
				cat $TMP_DIR/template_csi_nored_1_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_csi_nored_1_$TEAM_COMP_NAME
				
				cp ../IPA/templates/TEMPLATE_CSI/template_csi_nored_2 $TMP_DIR/template_csi_nored_2_$TEAM_COMP_NAME
				sed -i "s/#CSI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/template_csi_nored_2_$TEAM_COMP_NAME
				sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_csi_nored_2_$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_csi_nored_2_$TEAM_COMP_NAME
				sed -i "s/#CS_TYPE#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_csi_nored_2_$TEAM_COMP_NAME
				cat $TMP_DIR/template_csi_nored_2_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_csi_nored_2_$TEAM_COMP_NAME
			fi
			cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ] ; then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`
				HelthCheck=`echo $line_update | awk -F';' '{print $9}'`
				
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				
				cat templates/template_procinit_imm_open >> $TMP_DIR/final.xml
				if [ "$type" = "2N" ] ; then
					cp ../IPA/templates/TEMPLATE_CSI/template_csi_2n $TMP_DIR/template_csi_2n_$COMP_NAME
					sed -i "s/#SI_NAME#/$COMP_NAME/g" $TMP_DIR/template_csi_2n_$COMP_NAME
					sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_csi_2n_$COMP_NAME
					sed -i "s/#CSI_NAME#/$COMP_NAME/g" $TMP_DIR/template_csi_2n_$COMP_NAME
					sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_csi_2n_$COMP_NAME
					sed -i "s/#CS_TYPE#/$COMP_TYPE_NAME/g" $TMP_DIR/template_csi_2n_$COMP_NAME
					cat $TMP_DIR/template_csi_2n_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_csi_2n_$COMP_NAME
				else
					cp ../IPA/templates/TEMPLATE_CSI/template_csi_nored_1 $TMP_DIR/template_csi_nored_1_$COMP_NAME
					sed -i "s/#CSI_NAME#/$COMP_NAME/g" $TMP_DIR/template_csi_nored_1_$COMP_NAME
					sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_csi_nored_1_$COMP_NAME
					sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_csi_nored_1_$COMP_NAME
					sed -i "s/#CS_TYPE#/$COMP_TYPE_NAME/g" $TMP_DIR/template_csi_nored_1_$COMP_NAME
					cat $TMP_DIR/template_csi_nored_1_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_csi_nored_1_$COMP_NAME
					
					cp ../IPA/templates/TEMPLATE_CSI/template_csi_nored_2 $TMP_DIR/template_csi_nored_2_$COMP_NAME
					sed -i "s/#CSI_NAME#/$COMP_NAME/g" $TMP_DIR/template_csi_nored_2_$COMP_NAME
					sed -i "s/#SAF_APP#/ERIC-APG/g" $TMP_DIR/template_csi_nored_2_$COMP_NAME
					sed -i "s/#SAF_VERSION#/$SAF_VERSION/g" $TMP_DIR/template_csi_nored_2_$COMP_NAME
					sed -i "s/#CS_TYPE#/$COMP_TYPE_NAME/g" $TMP_DIR/template_csi_nored_2_$COMP_NAME
					cat $TMP_DIR/template_csi_nored_2_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_csi_nored_2_$COMP_NAME
				fi
				cat templates/template_procinit_imm_end >> $TMP_DIR/final.xml

				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SA AMF CSI CREATION DONE"
#END Creating SA AMF CSI part for 2N and NORED =====================================================================================================================================

	cat templates/template_pre_swadd_swremove-5 >> $TMP_DIR/final.xml

#START Creating swRemove part =====================================================================================================================================

	if [ "$OPT_TEAM" = $TRUE ] ; then
		#WA for Mismatch in Agent Configuration in sources.list
		if [ "$TEAM_COMP_NAME" = "Agent" ]
		then
			TEAM_COMP_NAME="AGENT"
		fi

		block_name=$(echo $TEAM_NEW_BUNDLE_NAME | awk -F'-' '{ print $2 }')
		versions_to_install="$(installationTagCheck $block_name)"
		if [ "$versions_to_install" != "" ] ; then
			cp -f templates/template_swremove_new_fbu_open $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME
		else
			cp -f templates/template_swaddremove_fbu_open $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME
		fi
		sed -i "s/#BFU_BUNDLE#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME
		cp templates/template_sw_remove_fbu $TMP_DIR/template_sw_remove_$TEAM_COMP_NAME
		BFU_BUNDLE_BASE_NAME=`echo "$TEAM_BUNDLE_BASE_NAME" | awk -F'-' '{print $1"-"$2}'`
		sed -i "s/#BFU_BUNDLE_BASE_NAME#/$BFU_BUNDLE_BASE_NAME-/g" $TMP_DIR/template_sw_remove_$TEAM_COMP_NAME
		sed -i "s@#PATHNAME_PREFIX#@$TEAM_COMP_PATH@g" $TMP_DIR/template_sw_remove_$TEAM_COMP_NAME
		#cat $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml

		#cat $TMP_DIR/template_sw_remove_$TEAM_COMP_NAME >> $TMP_DIR/final.xml

		#cat templates/template_swaddremove_fbu_end >> $TMP_DIR/final.xml
		rm -f $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME

		rm -f $TMP_DIR/template_sw_remove_$TEAM_COMP_NAME
	else
		while read line_update
		do
			FROM_RSTATE=`echo $line_update | awk -F';' '{print $3}'`
			BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
			COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
			COMP_PATH=`echo $line_update | awk -F';' '{print $4}'`
			COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
 			#WA for Mismatch in Agent Configuration in sources.list
			if [ "$COMP_NAME" = "Agent" ]
			then
				COMP_NAME="AGENT"
			fi
			BUNDLE_BASE_NAME=`echo "$BUNDLE_NAME"|sed 's/-[^-]*$//'`

			cp templates/template_sw_remove_fbu $TMP_DIR/template_sw_remove_$COMP_NAME
			BFU_BUNDLE_BASE_NAME=`echo "$BUNDLE_BASE_NAME" | awk -F'-' '{print $1"-"$2}'`
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$BFU_BUNDLE_BASE_NAME-/g" $TMP_DIR/template_sw_remove_$COMP_NAME

			sed -i "s@#PATHNAME_PREFIX#@$COMP_PATH@g" $TMP_DIR/template_sw_remove_$COMP_NAME

			BFU_SWREMOVE_TO_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
			
			block_name=$(echo $BUNDLE_NAME | awk -F'-' '{ print $2 }')
			versions_to_install="$(installationTagCheck $block_name)"
			if [ "$versions_to_install" != "" ] ; then
				cp templates/template_swremove_new_fbu_open $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME
				sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME
			else
				cp templates/template_swaddremove_fbu_open $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME
			fi
			
			sed -i "s/#BFU_BUNDLE#/$BUNDLE_NAME/g" $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME

			cat $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
			cat $TMP_DIR/template_sw_remove_$COMP_NAME >> $TMP_DIR/final.xml
			cat templates/template_swaddremove_fbu_end >> $TMP_DIR/final.xml
			
			rm -f $TMP_DIR/template_sw_remove_$COMP_NAME
			rm -f $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: SW REMOVE CREATION DONE"
	
#END Creating swRemove part ====================================================================================================================================


#START Creating swAdd part =====================================================================================================================================

	if [ "$OPT_TEAM" = $TRUE ] ; then
		#WA for Mismatch in Agent Configuration in sources.list
		if [ "$TEAM_COMP_NAME" = "Agent" ]
		then
			TEAM_COMP_NAME="AGENT"
		fi
		cp templates/template_sw_add-7 $TMP_DIR/template_sw_add_$TEAM_COMP_NAME
		sed -i "s/#NEW_BUNDLE_NAME#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_sw_add_$TEAM_COMP_NAME
		sed -i "s@#PATHNAME_PREFIX#@$TEAM_COMP_PATH@g" $TMP_DIR/template_sw_add_$TEAM_COMP_NAME

		cp templates/template_swaddremove_fbu_open $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME
		sed -i "s/#BFU_BUNDLE#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME
		cat $TMP_DIR/template_swaddremove_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
		cat $TMP_DIR/template_sw_add_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
		cat templates/template_swaddremove_fbu_end >> $TMP_DIR/final.xml
		rm -f $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME

		rm -f $TMP_DIR/template_sw_add_$TEAM_COMP_NAME
	else
		while read line_update
		do
			BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
			COMP_PATH=`echo $line_update | awk -F';' '{print $4}'`
			COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
			#WA for Mismatch in Agent Configuration in sources.list
			if [ "$COMP_NAME" = "Agent" ]
			then
				COMP_NAME="AGENT"
			fi
			cp templates/template_sw_add-7 $TMP_DIR/template_sw_add_$COMP_NAME
			sed -i "s@#PATHNAME_PREFIX#@$COMP_PATH@g" $TMP_DIR/template_sw_add_$COMP_NAME
			sed -i "s/#NEW_BUNDLE_NAME#/$BUNDLE_NAME/g" $TMP_DIR/template_sw_add_$COMP_NAME

			cp templates/template_swaddremove_fbu_open $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME
			sed -i "s/#BFU_BUNDLE#/$BUNDLE_NAME/g" $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME
			cat $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
			cat $TMP_DIR/template_sw_add_$COMP_NAME >> $TMP_DIR/final.xml
			cat templates/template_swaddremove_fbu_end >> $TMP_DIR/final.xml

			rm -f $TMP_DIR/template_swaddremove_fbu_open_$COMP_NAME
			rm -f $TMP_DIR/template_sw_add_$COMP_NAME
		done < $SOURCE_LIST_TMP
	fi
	cat templates/template_end_node_template-8 >> $TMP_DIR/final.xml
	is_verbose && echo "STEP: SW ADD CREATION DONE"
	
#END Creating swAdd part ==============================================================================================================================================


#START Creating entity template part ==================================================================================================================================

	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ]
		then
			cp templates/template_entity_template-9 $TMP_DIR/template_entity_template_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_entity_template_$TEAM_COMP_NAME
			sed -i "s/#NEW_RSTATE#/$OPT_TEAM_VERSION/g" $TMP_DIR/template_entity_template_$TEAM_COMP_NAME

			block_name=$(echo $TEAM_NEW_BUNDLE_NAME | awk -F'-' '{ print $2 }')
			versions_to_install="$(installationTagCheck $block_name)"
			if [ "$versions_to_install" != "" ] ; then
				cp templates/template_entitytemplate_fbu_open $TMP_DIR/template_entity_fbu_open_$TEAM_COMP_NAME
				sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_entity_fbu_open_$TEAM_COMP_NAME
			else
				cp templates/template_entity_fbu_open $TMP_DIR/template_entity_fbu_open_$TEAM_COMP_NAME
			fi
			sed -i "s/#BFU_BUNDLE#/$TEAM_NEW_BUNDLE_NAME/g" $TMP_DIR/template_entity_fbu_open_$TEAM_COMP_NAME
			#cat $TMP_DIR/template_entity_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			#cat $TMP_DIR/template_entity_template_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			#cat templates/template_entity_fbu_end >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_entity_fbu_open_$TEAM_COMP_NAME

			rm -f $TMP_DIR/template_entity_template_$TEAM_COMP_NAME
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			# 2N
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ]
			then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				cp templates/template_entity_template-9 $TMP_DIR/template_entity_template_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_entity_template_$COMP_NAME
				sed -i "s/#NEW_RSTATE#/$NEW_RSTATE/g" $TMP_DIR/template_entity_template_$COMP_NAME

				block_name=$(echo $BUNDLE_NAME | awk -F'-' '{ print $2 }')
				versions_to_install="$(installationTagCheck $block_name)"
				if [ "$versions_to_install" != "" ] ; then
					cp templates/template_entitytemplate_fbu_open $TMP_DIR/template_entity_fbu_open_$COMP_NAME
					sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_entity_fbu_open_$COMP_NAME
				else
					cp templates/template_entity_fbu_open $TMP_DIR/template_entity_fbu_open_$COMP_NAME
				fi

				sed -i "s/#BFU_BUNDLE#/$BUNDLE_NAME/g" $TMP_DIR/template_entity_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_entity_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				cat $TMP_DIR/template_entity_template_$COMP_NAME >> $TMP_DIR/final.xml
				cat templates/template_entity_fbu_end >> $TMP_DIR/final.xml

				rm -f $TMP_DIR/template_entity_fbu_open_$COMP_NAME
				rm -f $TMP_DIR/template_entity_template_$COMP_NAME
			fi
		done < $SOURCE_LIST_TMP
	fi
	cat templates/template_end_entity-10 >> $TMP_DIR/final.xml
	is_verbose && echo "STEP: ENTITY TEMPLATE CREATION DONE"
	

#END Creating entity template part =====================================================================================================================================


#START Supervision Period part =====================================================================================================================================
	cat templates/template_supervision_init >> $TMP_DIR/final.xml
	cp -f templates/template_supervision_breakpoint $TMP_DIR/template_supervision_breakpoint
	sed -i "s/#SUPERVISION_BREAKPOINT_TIMEOUT#/$SUPERVISION_BREAKPOINT_TIMEOUT/g" $TMP_DIR/template_supervision_breakpoint

	cp -f templates/template_supervision_script $TMP_DIR/template_supervision_script
	sed -i "s/#SUPERVISION_SCRIPT_TIMEOUT#/$SUPERVISION_SCRIPT_TIMEOUT/g" $TMP_DIR/template_supervision_script
	##cat $TMP_DIR/template_supervision_script >> $TMP_DIR/final.xml
	
	#copy supervision script under TMP folder
	cp -f $PATH_SCRIPTS/supervision.sh $TMP_DIR_OUT/supervision.sh
	rm -f $TMP_DIR/template_supervision_script
	is_verbose && echo "STEP: SUPERVISION PERIOD CREATION DONE"
	
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ] ; then

			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			cp templates/template_customtime_fbu_open $TMP_DIR/template_customtime_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name/g" $TMP_DIR/template_customtime_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_customtime_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml

			cat templates/template_customtime >> $TMP_DIR/final.xml

			cat templates/template_customtime_fbu_end >> $TMP_DIR/final.xml
			
			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			cp templates/template_callback_fbu_open $TMP_DIR/template_callback_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_callback_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_callback_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			
			if [ "$TEAM_HA_TYPE" = "2N" ] ; then
				cp templates/template_callback $TMP_DIR/template_callback_$TEAM_COMP_NAME
				sed -i "s/#SUPERVISION_SCRIPT_TIMEOUT#/$SUPERVISION_SCRIPT_TIMEOUT/g" $TMP_DIR/template_callback_$TEAM_COMP_NAME
				sed -i "s/#SI_NAME#/$TEAM_COMP_NAME/g" $TMP_DIR/template_callback_$TEAM_COMP_NAME
				cat $TMP_DIR/template_callback_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_callback_$TEAM_COMP_NAME
			else
				cp templates/template_callback $TMP_DIR/template_callback_$TEAM_COMP_NAME
				sed -i "s/#SUPERVISION_SCRIPT_TIMEOUT#/$SUPERVISION_SCRIPT_TIMEOUT/g" $TMP_DIR/template_callback_$TEAM_COMP_NAME
				sed -i "s/#SI_NAME#/$TEAM_COMP_NAME-1/g" $TMP_DIR/template_callback_$TEAM_COMP_NAME
				cat $TMP_DIR/template_callback_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_callback_$TEAM_COMP_NAME
				
				cat templates/template_callback_fbu_end >> $TMP_DIR/final.xml
				
				cat $TMP_DIR/template_customtime_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml

				cat templates/template_customtime >> $TMP_DIR/final.xml

				cat templates/template_customtime_fbu_end >> $TMP_DIR/final.xml
				
				cat $TMP_DIR/template_callback_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				
				cp templates/template_callback $TMP_DIR/template_callback_$TEAM_COMP_NAME
				sed -i "s/#SUPERVISION_SCRIPT_TIMEOUT#/$SUPERVISION_SCRIPT_TIMEOUT/g" $TMP_DIR/template_callback_$TEAM_COMP_NAME
				sed -i "s/#SI_NAME#/$TEAM_COMP_NAME-2/g" $TMP_DIR/template_callback_$TEAM_COMP_NAME
				cat $TMP_DIR/template_callback_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_callback_$TEAM_COMP_NAME
			fi

			cat templates/template_callback_fbu_end >> $TMP_DIR/final.xml
			
			rm -f $TMP_DIR/template_customtime_fbu_open_$TEAM_COMP_NAME
			rm -f $TMP_DIR/template_callback_fbu_open_$TEAM_COMP_NAME
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ] ; then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`
				HelthCheck=`echo $line_update | awk -F';' '{print $9}'`
				
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				cp templates/template_customtime_fbu_open $TMP_DIR/template_customtime_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name/g" $TMP_DIR/template_customtime_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_customtime_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				
				cat templates/template_customtime >> $TMP_DIR/final.xml

				cat templates/template_customtime_fbu_end >> $TMP_DIR/final.xml
				
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				cp templates/template_callback_fbu_open $TMP_DIR/template_callback_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_callback_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_callback_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				
				if [ "$type" = "2N" ] ; then
					cp templates/template_callback $TMP_DIR/template_callback_$COMP_NAME
					sed -i "s/#SUPERVISION_SCRIPT_TIMEOUT#/$SUPERVISION_SCRIPT_TIMEOUT/g" $TMP_DIR/template_callback_$COMP_NAME
					sed -i "s/#SI_NAME#/$COMP_NAME/g" $TMP_DIR/template_callback_$COMP_NAME
					cat $TMP_DIR/template_callback_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_callback_$COMP_NAME
				else
					cp templates/template_callback $TMP_DIR/template_callback_$COMP_NAME
					sed -i "s/#SUPERVISION_SCRIPT_TIMEOUT#/$SUPERVISION_SCRIPT_TIMEOUT/g" $TMP_DIR/template_callback_$COMP_NAME
					sed -i "s/#SI_NAME#/$COMP_NAME-1/g" $TMP_DIR/template_callback_$COMP_NAME
					cat $TMP_DIR/template_callback_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_callback_$COMP_NAME
					
					cat templates/template_callback_fbu_end >> $TMP_DIR/final.xml
				
					cat $TMP_DIR/template_customtime_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml

					cat templates/template_customtime >> $TMP_DIR/final.xml

					cat templates/template_customtime_fbu_end >> $TMP_DIR/final.xml
					
					cat $TMP_DIR/template_callback_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
					
					cp templates/template_callback $TMP_DIR/template_callback_$COMP_NAME
					sed -i "s/#SUPERVISION_SCRIPT_TIMEOUT#/$SUPERVISION_SCRIPT_TIMEOUT/g" $TMP_DIR/template_callback_$COMP_NAME
					sed -i "s/#SI_NAME#/$COMP_NAME-2/g" $TMP_DIR/template_callback_$COMP_NAME
					cat $TMP_DIR/template_callback_$COMP_NAME >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_callback_$COMP_NAME
				fi

				cat templates/template_callback_fbu_end >> $TMP_DIR/final.xml
				
				rm -f $TMP_DIR/template_customtime_fbu_open_$COMP_NAME
				rm -f $TMP_DIR/template_callback_fbu_open_$COMP_NAME
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: ACTIVATE_SI CREATION DONE"
	
	cat $TMP_DIR/template_supervision_breakpoint >> $TMP_DIR/final.xml
	rm -f $TMP_DIR/template_supervision_breakpoint
	cat templates/template_supervision_end >> $TMP_DIR/final.xml

#END Supervision Period part =====================================================================================================================================

#START Creating wrapup action part =====================================================================================================================================
	if [ "$IS_MIXED_UPDATE" = "1" ]
	then
		if [ "$OPT_TEAM" = $TRUE ] ; then

			if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ] ; then
				block_name=$(echo $TEAM_NEW_BUNDLE_NAME | awk -F'-' '{ print $2 }')
				versions_to_install="$(installationTagCheck $block_name)"
				if [ "$versions_to_install" != "" ] ; then
					cp templates/template_procwrapup_new_fbu_open $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
					sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
				else
					cp templates/template_wrapup_fbu_open $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
				fi
				sed -i "s/#NEW_RSTATE#/$NEW_RSTATE/g" $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION_SU#/$SAF_VERSION/g" $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
				if [ "$TEAM_HA_TYPE" = "2N" ] ; then
					sed -i "s/#SU_TYPE#/$SU_2N_TYPE/g" $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
				else
					sed -i "s/#SU_TYPE#/ERIC-APG_SU_$TEAM_CSI_NAME/g" $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
				fi
				##cat $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_wrapup_fbu_open_$TEAM_COMP_NAME
			fi

			##cat templates/template_wrapup_action_init-11 >> $TMP_DIR/final.xml

			if [ "$TEAM_HA_TYPE" = "2N" ]
			then	
				cp -f templates/template_delete-12a $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				cp -f templates/template_delete-12b $TMP_DIR/template_delete_b$TEAM_COMP_NAME
				cp -f templates/template_delete-12c $TMP_DIR/template_delete_c$TEAM_COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				sed -i "s/#SU_TYPE#/$SU_2N_TYPE/g" $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION_SU#/$SAF_VERSION/g" $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_delete_b$TEAM_COMP_NAME
				sed -i "s/#HCK#/$TEAM_HelthCheck/g" $TMP_DIR/template_delete_b$TEAM_COMP_NAME
				sed -i "s/#CS_TYPE#/$TEAM_CS_TYPE/g" $TMP_DIR/template_delete_c$TEAM_COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_delete_c$TEAM_COMP_NAME
				#cat $TMP_DIR/template_delete_a$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				#cat $TMP_DIR/template_delete_b$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				#cat $TMP_DIR/template_delete_c$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				rm -f $TMP_DIR/template_delete_b$TEAM_COMP_NAME
				rm -f $TMP_DIR/template_delete_c$TEAM_COMP_NAME
			fi	
			# NORED
			if [ "$TEAM_HA_TYPE" = "NORED" ]
			then		
				cp -f templates/template_delete-12a $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				cp -f templates/template_delete-12b $TMP_DIR/template_delete_b$TEAM_COMP_NAME
				cp -f templates/template_delete-12c $TMP_DIR/template_delete_c$TEAM_COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				sed -i "s/#SU_TYPE#/ERIC-APG_SU_$TEAM_CSI_NAME/g" $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				sed -i "s/#SAF_VERSION_SU#/$SAF_VERSION/g" $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_delete_b$TEAM_COMP_NAME
				sed -i "s/#HCK#/$TEAM_HelthCheck/g" $TMP_DIR/template_delete_b$TEAM_COMP_NAME
				sed -i "s/#CS_TYPE#/$TEAM_CS_TYPE/g" $TMP_DIR/template_delete_c$TEAM_COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_delete_c$TEAM_COMP_NAME
				##cat $TMP_DIR/template_delete_a$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				##cat $TMP_DIR/template_delete_b$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				##cat $TMP_DIR/template_delete_c$TEAM_COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_delete_a$TEAM_COMP_NAME
				rm -f $TMP_DIR/template_delete_b$TEAM_COMP_NAME
				rm -f $TMP_DIR/template_delete_c$$TEAM_COMP_NAME	
			fi
			##cat templates/template_wrapup_action_end-13 >> $TMP_DIR/final.xml
			
			if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ] ; then
				##cat templates/template_wrapup_fbu_end >> $TMP_DIR/final.xml
				echo -n -e ""
			fi
		else
			while read line_update
			do
				type=`echo $line_update | awk -F';' '{print $1}'`
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				# 2N
				if [ "$type" = "2N" ]
				then
					COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
					COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
					HelthCheck=`echo $line_update | awk -F';' '{print $9}'`
					CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
					cp -f templates/template_delete-12a $TMP_DIR/template_delete_a$COMP_NAME
					cp -f templates/template_delete-12b $TMP_DIR/template_delete_b$COMP_NAME
					cp -f templates/template_delete-12c $TMP_DIR/template_delete_c$COMP_NAME
					sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_delete_a$COMP_NAME
					sed -i "s/#SU_TYPE#/$SU_2N_TYPE/g" $TMP_DIR/template_delete_a$COMP_NAME
					sed -i "s/#SAF_VERSION_SU#/$SAF_VERSION/g" $TMP_DIR/template_delete_a$COMP_NAME
					sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_delete_b$COMP_NAME
					sed -i "s/#HCK#/$HelthCheck/g" $TMP_DIR/template_delete_b$COMP_NAME
					sed -i "s/#CS_TYPE#/$CS_TYPE/g" $TMP_DIR/template_delete_c$COMP_NAME
					sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_delete_c$COMP_NAME

					NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`

					block_name=$(echo $BUNDLE_NAME | awk -F'-' '{ print $2 }')
					versions_to_install="$(installationTagCheck $block_name)"
					if [ "$versions_to_install" != "" ] ; then
						cp templates/template_procwrapup_new_fbu_open $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
						sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					else
						cp templates/template_wrapup_fbu_open $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					fi

					sed -i "s/#NEW_RSTATE#/$NEW_RSTATE/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					sed -i "s/#SAF_VERSION_SU#/$SAF_VERSION/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					sed -i "s/#SU_TYPE#/$SU_2N_TYPE/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME

					cat $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
					
					cat templates/template_wrapup_action_init-11 >> $TMP_DIR/final.xml
					cat $TMP_DIR/template_delete_a$COMP_NAME >> $TMP_DIR/final.xml
					cat $TMP_DIR/template_delete_b$COMP_NAME >> $TMP_DIR/final.xml
					cat $TMP_DIR/template_delete_c$COMP_NAME >> $TMP_DIR/final.xml
					cat templates/template_wrapup_action_end-13 >> $TMP_DIR/final.xml

					cat templates/template_wrapup_fbu_end >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME

					rm -f $TMP_DIR/template_delete_a$COMP_NAME
					rm -f $TMP_DIR/template_delete_b$COMP_NAME
					rm -f $TMP_DIR/template_delete_c$COMP_NAME
				fi
				# NORED
				if [ "$type" = "NORED" ]
				then
					COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
					COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
					CSI_NAME=`echo $line_update | awk -F';' '{print $11}'`
					HelthCheck=`echo $line_update | awk -F';' '{print $9}'`
					CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
					cp -f templates/template_delete-12a $TMP_DIR/template_delete_a$COMP_NAME
					cp -f templates/template_delete-12b $TMP_DIR/template_delete_b$COMP_NAME
					cp -f templates/template_delete-12c $TMP_DIR/template_delete_c$COMP_NAME
					sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_delete_a$COMP_NAME
					sed -i "s/#SU_TYPE#/ERIC-APG_SU_$CSI_NAME/g" $TMP_DIR/template_delete_a$COMP_NAME
					sed -i "s/#SAF_VERSION_SU#/$SAF_VERSION/g" $TMP_DIR/template_delete_a$COMP_NAME
					sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_delete_b$COMP_NAME
					sed -i "s/#HCK#/$HelthCheck/g" $TMP_DIR/template_delete_b$COMP_NAME
					sed -i "s/#CS_TYPE#/$CS_TYPE/g" $TMP_DIR/template_delete_c$COMP_NAME
					sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_delete_c$COMP_NAME

					NEW_RSTATE=`echo $BUNDLE_NAME | awk -F'-' '{print $4}'`

					block_name=$(echo $BUNDLE_NAME | awk -F'-' '{ print $2 }')
					versions_to_install="$(installationTagCheck $block_name)"
					if [ "$versions_to_install" != "" ] ; then
						cp templates/template_procwrapup_new_fbu_open $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
						sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					else
						cp templates/template_wrapup_fbu_open $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					fi

					sed -i "s/#NEW_RSTATE#/$NEW_RSTATE/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					sed -i "s/#SAF_VERSION_SU#/$SAF_VERSION/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME
					sed -i "s/#SU_TYPE#/ERIC-APG_SU_$CSI_NAME/g" $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME

					cat $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml

					cat templates/template_wrapup_action_init-11 >> $TMP_DIR/final.xml
					cat $TMP_DIR/template_delete_a$COMP_NAME >> $TMP_DIR/final.xml
					cat $TMP_DIR/template_delete_b$COMP_NAME >> $TMP_DIR/final.xml
					cat $TMP_DIR/template_delete_c$COMP_NAME >> $TMP_DIR/final.xml
					cat templates/template_wrapup_action_end-13 >> $TMP_DIR/final.xml

					cat templates/template_wrapup_fbu_end >> $TMP_DIR/final.xml
					rm -f $TMP_DIR/template_wrapup_fbu_open_$COMP_NAME

					rm -f $TMP_DIR/template_delete_a$COMP_NAME
					rm -f $TMP_DIR/template_delete_b$COMP_NAME
					rm -f $TMP_DIR/template_delete_c$COMP_NAME	
				fi
			done < $SOURCE_LIST_TMP
		fi
		is_verbose && echo "STEP: WRAPUP CREATION DONE"
	fi

#END Creating wrapup action part ==============================================================================================================================================

#START Creating activate_su part ==============================================================================================================================================
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "NORED" ] ; then
			block_name=$(echo $TEAM_CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
			new_bundle_base_name=`echo "$TEAM_NEW_BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
			
			cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			
			cp templates/template_activate_su $TMP_DIR/template_activate_su_$TEAM_COMP_NAME
			sed -i "s/#SU_NR#/2/g" $TMP_DIR/template_activate_su_$TEAM_COMP_NAME
			sed -i "s/#BLOCK_NAME#/$block_name/g" $TMP_DIR/template_activate_su_$TEAM_COMP_NAME
			cat $TMP_DIR/template_activate_su_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_activate_su_$TEAM_COMP_NAME
			
			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			
			cat $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_procinit_installed_fbu_open_$TEAM_COMP_NAME
			
			cp templates/template_activate_su $TMP_DIR/template_activate_su_$TEAM_COMP_NAME
			sed -i "s/#SU_NR#/1/g" $TMP_DIR/template_activate_su_$TEAM_COMP_NAME
			sed -i "s/#BLOCK_NAME#/$block_name/g" $TMP_DIR/template_activate_su_$TEAM_COMP_NAME
			cat $TMP_DIR/template_activate_su_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_activate_su_$TEAM_COMP_NAME
			
			cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
		fi
	else
		while read line_update ; do
			type=`echo $line_update | awk -F';' '{print $1}'`
			if [ "$type" = "NORED" ] ; then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				CS_TYPE=`echo $line_update | awk -F';' '{print $6}'`
				block_name=$(echo $CS_TYPE | awk -F'-' '{ print $2 }' | awk -F'_' '{ print $2 }')
				new_bundle_base_name=`echo "$BUNDLE_NAME" | awk -F'-' '{print $1"-"$2}'`
				
				cp templates/template_procinit_installed_fbu_open $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				sed -i "s/#BFU_BUNDLE_BASE_NAME#/$new_bundle_base_name-/g" $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				
				cp templates/template_activate_su $TMP_DIR/template_activate_su_$COMP_NAME
				sed -i "s/#SU_NR#/2/g" $TMP_DIR/template_activate_su_$COMP_NAME
				sed -i "s/#BLOCK_NAME#/$block_name/g" $TMP_DIR/template_activate_su_$COMP_NAME
				cat $TMP_DIR/template_activate_su_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_activate_su_$COMP_NAME
				
				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
				
				cat $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_procinit_installed_fbu_open_$COMP_NAME
				
				cp templates/template_activate_su $TMP_DIR/template_activate_su_$COMP_NAME
				sed -i "s/#SU_NR#/1/g" $TMP_DIR/template_activate_su_$COMP_NAME
				sed -i "s/#BLOCK_NAME#/$block_name/g" $TMP_DIR/template_activate_su_$COMP_NAME
				cat $TMP_DIR/template_activate_su_$COMP_NAME >> $TMP_DIR/final.xml
				rm -f $TMP_DIR/template_activate_su_$COMP_NAME
				
				cat templates/template_procinit_installed_fbu_end >> $TMP_DIR/final.xml
			fi
		done < $SOURCE_LIST_TMP
	fi
	is_verbose && echo "STEP: ACTIVATE_SU CREATION DONE"
#END Creating activate_su part ================================================================================================================================================

#START Creating remove from imm part =====================================================================================================================================

	cat templates/template_upgrade_procedure_end-14 >> $TMP_DIR/final.xml
	cat templates/template_campaign_wrapup_init-15 >> $TMP_DIR/final.xml
	#Verify if baseline files are present
	if [ "$OPT_TEAM" = $TRUE ] ; then
		is_verbose && echo "STEP: BASELINE NOT CREATED IN TEAM UPGRADE PACKAGE"
	else
		if [ ! -f "$PATH_SCRIPTS/$BASELINE_FILE_GEP12_AP1" ];then
			echo "ERROR: Baseline File for GEP1/GEP2 AP1 Not Found!"
			exit 1
		fi
		if [ ! -f "$PATH_SCRIPTS/$BASELINE_FILE_GEP12_AP2" ];then
			echo "ERROR: Baseline File for GEP1/GEP2 AP2 Not Found!"
			exit 1
		fi
		if [ ! -f "$PATH_SCRIPTS/$BASELINE_FILE_GEP5_AP1" ];then
			echo "ERROR: Baseline File for GEP5 AP1 Not Found!"
			exit 1
		fi
		if [ ! -f "$PATH_SCRIPTS/$BASELINE_FILE_GEP5_AP2" ];then
			echo "ERROR: Baseline File for GEP5 AP2 Not Found!"
			exit 1
		fi
		if [ "$AP2_CAMP" = "$TRUE" ];then
			BASELINE_FILE_GEP12_AP2_FINAL=$(echo $BASELINE_FILE_GEP12_AP2 | cut -c5-)
			BASELINE_FILE_GEP5_AP2_FINAL=$(echo $BASELINE_FILE_GEP5_AP2 | cut -c5-)
			cp -f $PATH_SCRIPTS/$BASELINE_FILE_GEP12_AP2 $TMP_DIR_OUT/$BASELINE_FILE_GEP12_AP2_FINAL
			cp -f $PATH_SCRIPTS/$BASELINE_FILE_GEP5_AP2 $TMP_DIR_OUT/$BASELINE_FILE_GEP5_AP2_FINAL
		else
			BASELINE_FILE_GEP12_AP1_FINAL=$(echo $BASELINE_FILE_GEP12_AP1 | cut -c5-)
			BASELINE_FILE_GEP5_AP1_FINAL=$(echo $BASELINE_FILE_GEP5_AP1 | cut -c5-)
			cp -f $PATH_SCRIPTS/$BASELINE_FILE_GEP12_AP1 $TMP_DIR_OUT/$BASELINE_FILE_GEP12_AP1_FINAL
			cp -f $PATH_SCRIPTS/$BASELINE_FILE_GEP5_AP1 $TMP_DIR_OUT/$BASELINE_FILE_GEP5_AP1_FINAL
		fi
		cp -f templates/template_baseline $TMP_DIR/template_baseline
		sed -i "s/#BASELINE_SCRIPTS#/$BASELINE_SCRIPTS/g" $TMP_DIR/template_baseline
		cat $TMP_DIR/template_baseline >> $TMP_DIR/final.xml
		cp -f $PATH_SCRIPTS/$BASELINE_SCRIPTS $TMP_DIR_OUT/
	fi
	cat templates/template_waitTo >> $TMP_DIR/final.xml
	##cat templates/template_remove_from_imm_init-16 >> $TMP_DIR/final.xml
	if [ "$OPT_TEAM" = $TRUE ] ; then
		if [ "$TEAM_HA_TYPE" = "2N" ] || [ "$TEAM_HA_TYPE" = "NORED" ]
		then
			cp templates/template_amf_entity-17 $TMP_DIR/template_amf_entity_$TEAM_COMP_NAME
			sed -i "s/#COMP_TYPE_NAME#/$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_amf_entity_$TEAM_COMP_NAME

			block_name=$(echo $TEAM_NEW_BUNDLE_NAME | awk -F'-' '{ print $2 }')
			versions_to_install="$(installationTagCheck $block_name)"
			if [ "$versions_to_install" != "" ] ; then
				cp templates/template_amfentitytype_new_fbu_open $TMP_DIR/template_amfentity_fbu_open_$TEAM_COMP_NAME
				sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_amfentity_fbu_open_$TEAM_COMP_NAME
			else
				cp templates/template_amfentity_fbu_open $TMP_DIR/template_amfentity_fbu_open_$TEAM_COMP_NAME
			fi
			rstate="$(echo $TEAM_NEW_BUNDLE_NAME | awk -F'-' '{print $4}')"
			sed -i "s/#BFU_BUNDLE#/safVersion=$rstate,safCompType=$TEAM_COMP_TYPE_NAME/g" $TMP_DIR/template_amfentity_fbu_open_$TEAM_COMP_NAME
			#cat $TMP_DIR/template_amfentity_fbu_open_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			#cat $TMP_DIR/template_amf_entity_$TEAM_COMP_NAME >> $TMP_DIR/final.xml
			#cat templates/template_amfentity_fbu_end >> $TMP_DIR/final.xml
			rm -f $TMP_DIR/template_amfentity_fbu_open_$TEAM_COMP_NAME

			rm -f $TMP_DIR/template_amf_entity_$TEAM_COMP_NAME
		fi
	else
		while read line_update
		do
			type=`echo $line_update | awk -F';' '{print $1}'`
			# 2N
			if [ "$type" = "2N" ] || [ "$type" = "NORED" ]
			then
				BUNDLE_NAME=`echo $line_update | awk -F';' '{print $2}'`
				COMP_TYPE_NAME=`echo $line_update | awk -F';' '{print $5}'`
				COMP_NAME=`echo $line_update | awk -F';' '{print $3}'`
				cp templates/template_amf_entity-17 $TMP_DIR/template_amf_entity_$COMP_NAME
				sed -i "s/#COMP_TYPE_NAME#/$COMP_TYPE_NAME/g" $TMP_DIR/template_amf_entity_$COMP_NAME

				rstate="$(echo $BUNDLE_NAME | awk -F'-' '{print $4}')"
				block_name=$(echo $BUNDLE_NAME | awk -F'-' '{ print $2 }')
				versions_to_install="$(installationTagCheck $block_name)"
				if [ "$versions_to_install" != "" ] ; then
					cp templates/template_amfentitytype_new_fbu_open $TMP_DIR/template_amfentity_fbu_open_$COMP_NAME
					sed -i "s/#SW_VERSION#/($versions_to_install).(.)+.(.)+-(.)+/g" $TMP_DIR/template_amfentity_fbu_open_$COMP_NAME
				else
					cp templates/template_amfentity_fbu_open $TMP_DIR/template_amfentity_fbu_open_$COMP_NAME
				fi
				sed -i "s/#BFU_BUNDLE#/safVersion=$rstate,safCompType=$COMP_TYPE_NAME/g" $TMP_DIR/template_amfentity_fbu_open_$COMP_NAME
				cat $TMP_DIR/template_amfentity_fbu_open_$COMP_NAME >> $TMP_DIR/final.xml
				cat $TMP_DIR/template_amf_entity_$COMP_NAME >> $TMP_DIR/final.xml
				cat templates/template_amfentity_fbu_end >> $TMP_DIR/final.xml

				rm -f $TMP_DIR/template_amf_entity_$COMP_NAME
				rm -f $TMP_DIR/template_amfentity_fbu_open_$COMP_NAME
			fi
		done < $SOURCE_LIST_TMP
	fi
	#cat templates/template_remove_from_imm_end-18 >> $TMP_DIR/final.xml
echo "<removeFromImm />" >> $TMP_DIR/final.xml
	is_verbose && echo "STEP: REMOVE FROM IMM CREATION DONE"
	
#END Creating remove from imm part =============================================================================================================================


#START final part ==============================================================================================================================================

	cat templates/template_end-19 >> $TMP_DIR/final.xml
	is_verbose && echo "STEP: FINAL CREATION DONE"

#END final part ================================================================================================================================================


#START ETF part ================================================================================================================================================

	cp -f templates/template_etf_campaign $TMP_DIR/ETF.xml
	sed -i "s/#UP_CAMPAIGN_NAME#/$UP_CAMPAIGN_NAME/g" $TMP_DIR/ETF.xml
	cp -f $TMP_DIR/ETF.xml $TMP_DIR_OUT/ETF.xml
	is_verbose && echo "STEP: ETF CREATION DONE"

#END ETF part ================================================================================================================================================


#START CAMPAIGN SDP part =====================================================================================================================================
	
	cp -f $PATH_SCRIPTS/apos_drbdstatus.sh $TMP_DIR_OUT/apos_drbdstatus.sh
	# Copied apos_drbdstatus.sh file

	cp -f $PATH_SCRIPTS/model_changes.sh $TMP_DIR_OUT/model_changes.sh
	cp -f $PATH_SCRIPTS/activate_si.sh $TMP_DIR_OUT/activate_si.sh
	cp -f $PATH_SCRIPTS/activate_su.sh $TMP_DIR_OUT/activate_su.sh

	cp $TMP_DIR/final.xml $TMP_DIR_OUT/campaign.template.xml

	which tidy &> /dev/null
	if [ $? -eq 0 ]; then
		tidy -output $TMP_DIR_OUT/campaign.template.xml -indent -wrap -clean -xml -asxml -ascii $TMP_DIR_OUT/campaign.template.xml &> /dev/null
		tidy -output $TMP_DIR_OUT/ETF.xml -indent -wrap -clean -xml -asxml -utf8 $TMP_DIR_OUT/ETF.xml &> /dev/null
	fi

	pushd $TMP_DIR_OUT >> /dev/null
	chmod 775 ./apos_drbdstatus.sh

	chmod 775 ./model_changes.sh
	chmod 775 ./activate_si.sh
	chmod 775 ./activate_su.sh

	# we are in tmp folder and we can ignore relative path
	chmod 775 ./supervision.sh

        if [ "$OPT_TEAM" = $TRUE ] ; then
                tar -czf $UP_CAMPAIGN_FILE_NAME ETF.xml campaign.template.xml supervision.sh apos_drbdstatus.sh model_changes.sh activate_si.sh activate_su.sh
                if [ "$?" != "0"  ]
                then
                        echo "ERROR Failed to create Campaign SDP"
                        exit 29
                fi
                rm -f ETF.xml campaign.template.xml supervision.sh apos_drbdstatus.sh model_changes.sh activate_si.sh activate_su.sh
	else
                chmod 775 ./$BASELINE_SCRIPTS
                tar -czf $UP_CAMPAIGN_FILE_NAME ETF.xml campaign.template.xml supervision.sh apos_drbdstatus.sh model_changes.sh activate_si.sh activate_su.sh *_acs_lct_sha1verify.conf $BASELINE_SCRIPTS
                if [ "$?" != "0"  ]
                then
                        echo "ERROR Failed to create Campaign SDP"
                        exit 29
                fi
                rm -f ETF.xml campaign.template.xml supervision.sh apos_drbdstatus.sh model_changes.sh activate_si.sh activate_su.sh *_acs_lct_sha1verify.conf $BASELINE_SCRIPTS
	fi
	popd >> /dev/null

	is_verbose && echo "STEP: CAMPAIGN SDP CREATION DONE"

}

#END CAMPAIGN SDP part ===========================================================================================================================================

#Verify if AP2 update is to be executed
function update_dual_ap () {
	if [ "$OPT_TEAM" = "$FALSE" ]; then
		cp -f $TMP_DIR/free.list $TMP_DIR/list4PackageInfo
		if [ -e $PATH_SOURCE_FILE/$AP2_FILE_NAME ] ; then 
			if [ -e $TMP_DIR/free.list ]; then
				while read line 
				do	
					[ -n "$line" ] && sed -i "/$line/ s/^/#/g" $TMP_DIR/free.list
				done < $PATH_SOURCE_FILE/$AP2_FILE_NAME
			fi
		else 
			echo "ERROR: $AP2_FILE_NAME NOT FOUND! "
			exit 2
		fi
		
		#Check that there is at least an AP2 bundle
		sed -i "s/^#.*$//g" $TMP_DIR/free.list
		sed -i '/^$/d' $TMP_DIR/free.list
		UPDATE_CHECK=`cat $TMP_DIR/free.list| wc -l`
		
		if [ "$UPDATE_CHECK" = "0" ]; then
			echo "INFO: no bundles to be updated on AP2 "
		else 	
			AP2_CAMP=$TRUE
			is_verbose && echo "INFO: AP2 UPDATE TO BE EXECUTED"
			makeCampaign $UP_CAMPAIGN_NAME_2
		fi
	else
		AP2_CAMP=$TRUE
		is_verbose && echo "INFO: CHECKING IF AP2 UPDATE IS NEEDED"
		
		while read line 
		do 
			if [ "$line" = "$UP_ITEM" ]; then
				AP2_CAMP=$FALSE
				break
			fi
		done < $PATH_SOURCE_FILE/$AP2_FILE_NAME
		if [ "$AP2_CAMP" = "$TRUE" ]; then
			is_verbose && echo "INFO: AP2 UPDATE NEEDED "
			makeCampaign $UP_CAMPAIGN_NAME_2
		else
			is_verbose && echo "INFO: AP2 UPDATE NOT NEEDED "
		fi
	fi
}
#######################################################################
#       		 MAIN	 		  		      #			
#######################################################################
makeCampaign $UP_CAMPAIGN_NAME_1
update_dual_ap
	
#START PACKAGE INFO AND SHA256 part ==============================================================================================================================

createPackageInfo
createChSum
is_verbose && echo "STEP: PACKAGE INFO AND SHA256 CREATION DONE"

#END PACKAGE INFO AND SHA256 part ================================================================================================================================

#START UP FILE part ==============================================================================================================================================
if [ "$OPT_TEAM" = $TRUE ] ; then
	if [ -e $OPT_TEAM_SDP_PATH/$UP_NAME ] ; then
		rm -f $OPT_TEAM_SDP_PATH/$UP_NAME
		if [ "$?" != "0"  ]
		then
			echo "ERROR Failed to remove old UP file"
			exit 26
		fi
	fi
fi
pushd $TMP_DIR_OUT >> /dev/null

FILES_LIST=$(find *)
tar -czf $UP_NAME $FILES_LIST
if [ "$?" != "0"  ]
then
	echo "ERROR Failed to create UP"
	exit 26
fi
popd >> /dev/null
if [ "$OPT_TEAM" = $TRUE ] ; then
	cp -f $TMP_DIR_OUT/$UP_NAME $OPT_TEAM_SDP_PATH/$UP_NAME
else
	cp -f $TMP_DIR_OUT/$UP_NAME $PATH_OUTPUT_FOLDER/$UP_NAME
fi
is_verbose && echo "STEP: UP FILE CREATION DONE"
	
#END UP FILE part ==============================================================================================================================================

rm -rf $TMP_DIR
