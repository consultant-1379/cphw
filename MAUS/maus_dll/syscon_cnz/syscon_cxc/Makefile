# **********************************************************************
#
# Short description:
# Makefile for syscon_cxc
# **********************************************************************
#
# Ericsson AB 2010 All rights reserved.
# The information in this document is the property of Ericsson.
# Except as specifically authorized in writing by Ericsson, the receiver of this
# document shall keep the information contained herein confidential and shall protect
# the same in whole or in part from disclosure and dissemination to third parties.
# Disclosure and disseminations to the receivers employees shall only be made
# on a strict need to know basis.
#
# **********************************************************************
#
# Rev        Date         Name      What
# -----      ----------   --------  --------------------------
#            2015-11-19   xdtthng   Created
#
# **********************************************************************
 
# BEGIN: GIT INTRO.
CURDIR = $(shell pwd)
REPO_NAME = cphw
CPHW_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(CPHW_ROOT)/common

include $(COMMON_ROOT)/common.mk 

 

CXC_NAME    ?= CPHW_SYSCONBIN
CXC_NR      ?= CXC1372150_10
CXC_VER     ?= R1B
CXC_OLD_VER ?= 
CXC_BLK_NAME ?= cphw_syscon
BASE_SW_VER ?= 3.7.0-R1A
FX_CLEAN_TMP := rm -rf /tmp/$(USER)/*
#ESM_VER = $(shell $(ESM_SCRIPTS_DIR)/getversion.sh $(CXC_NR) $(CXC_VER))
CURDIR  = $(shell pwd)
CXCPATH = $(CURDIR)
CXCDIR  = $(CXCPATH)
BINDIR	= $(CXCPATH)/bin
APPNAME = syscon
APPNAME_DBG = syscon_dbg
# Define SYSCON app name
SYSCON_APNAME   = cphw_syscond
SYSCON_CMD_APNAME  = syscon
SYSLOGLS_CMD_APNAME = syslogls

CAA_DIRS = ../syscon_cmd_caa
			 

SYSCON_DIR				= ../syscon_adm_caa
SYSCON_COMMON_DIR		= ../syscon_common
SYSCON_CMD_DIR 			= ../syscon_cmd_caa/syscon
SYSLOG_CMD_DIR			= ../syscon_cmd_caa/syslogls
SYSCON_CMD_COMMON_DIR 	= ../syscon_cmd_caa/common

# Define CPPCHECK src and inc directories
CPPCKECK_SRCS = $(SYSCON_DIR)/src/*.cpp \
				$(SYSCON_COMMON_DIR)/src/*.cpp \
				$(SYSCON_CMD_DIR)/src/*.cpp \
				$(SYSLOG_CMD_DIR)/src/*.cpp \
				$(SYSCON_CMD_COMMON_DIR)/src/*.cpp 

CPPCKECK_INCS = -I$(SYSCON_DIR)/inc \
				-I$(SYSCON_COMMON_DIR)/inc \
				-I$(SYSCON_CMD_DIR)/inc \
				-I$(SYSLOG_CMD_DIR)/inc \
				-I$(SYSCON_CMD_COMMON_DIR)/inc 

# Define CCCC src
CCCC_SRCS = $(SYSCON_DIR)/src/* \
			$(SYSCON_COMMON_DIR)/src/* \
			$(SYSCON_CMD_DIR)/src/* \
			$(SYSLOG_CMD_DIR)/src/* \
			$(SYSCON_CMD_COMMON_DIR)/src/* 

# Define SLOCCOUNT src
SLOCCOUNT_SRCS = $(SYSCON_DIR) $(SYSCON_COMMON_DIR) $(SYSCON_CMD_DIR) $(SYSLOG_CMD_DIR) $(SYSCON_CMD_COMMON_DIR) 

# TODO: no need for now, do later
DOXYGEN_OUTPUT = $(DOCDIR)/docgen
BLOCK_NAME     = $(CXC_NAME)
BLOCK_VERSION  = $(CXC_VER)
BLOCK_SEARCH  = $(SYSCON_DIR)/inc $(SYSCON_DIR)/src $(SYSCON_COMMON_DIR)/inc $(SYSCON_COMMON_DIR)/src $(SYSCON_CMD_DIR)/inc $(SYSCON_CMD_DIR)/src \
				$(SYSLOG_CMD_DIR)/inc $(SYSLOG_CMD_DIR)/src $(SYSCON_CMD_COMMON_DIR)/inc $(SYSCON_CMD_COMMON_DIR)/src
				 
export DOXYGEN_OUTPUT
export BLOCK_NAME
export BLOCK_VERSION
export BLOCK_SEARCH

.PHONY: all clean distclean
all clean distclean: 
	$(call FX_CLEAN_TMP)
	@for i in $(CAA_DIRS) ; do \
		(echo "cd $$i" ; \
		 cd $$i ; \
		 $(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) CPHW_ROOT=$(CPHW_ROOT) $@) ;\
	done
	$(call ct_co,$(BINDIR)/$(APPNAME))	
	$(call ct_co,$(BINDIR)/.debug/$(APPNAME_DBG))	

	$(call ct_ci,$(BINDIR)/$(APPNAME))	
	$(call ct_ci,$(BINDIR)/.debug/$(APPNAME_DBG))	

	$(call ct_unco,$(BINDIR)/$(APPNAME))	
	$(call ct_unco,$(BINDIR)/.debug/$(APPNAME_DBG))	

.PHONY: rpm_package
rpm_package:
	$(call rpm_file_creation,syscon.spec,$(CXC_NAME),$(CXC_NR),$(CXC_VER))

.PHONY: sdp_package
sdp_package:
	$(SILENT)$(ECHO) 'SDP packaging ...'
	$(call sdp_creation_US1,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(CXC_OLD_VER),$(CXC_NR))
	$(call up_creation,$(CXC_NAME),$(CXC_NR),$(CXC_VER),$(CXC_OLD_VER),$(CXCPATH),$(BASE_SW_VER))
.PHONY: esm_package
esm_package:
	$(call esm_pkg_creation,$(CXCPATH),$(CXC_BLK_NAME),$(CXC_VER),$(BASE_SW_VER),$(CXC_NR),$(CXC_NAME))
                      
.PHONY: upload_package 
upload_package: 
	$(call upload_package,$(CXC_BLK_NAME),$(CXC_NR),$(CXC_NAME),$(CXC_VER))

.PHONY: tar_package
tar_package:
	$(call tar_creation,$(BLOCK_NAME),$(CXC_NR),$(CXC_VER))

.PHONY: nightbuild
nightbuild: del_update  distclean all rpm_package sdp_package esm_package 

.PHONY: del_update
del_update:
	$(call rpm_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call sdp_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call esm_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_ESM_VER),$(CXC_NR))

.PHONY: release
release: all rpm_package sdp_package esm_package tar_package documentation metrics
	
.PHONY: metrics
metrics: cppcheck_m cccc_m sloccount_m

.PHONY: documentation
documentation:
	$(call doc_creation)

.PHONY: cppcheck_m
cppcheck_m:
	$(call cppcheck_doc_creation,$(CPPCKECK_SRCS),$(CPPCKECK_INCS))

.PHONY: cccc_m
cccc_m:		
	$(call cccc_doc_creation,$(CCCC_SRCS))

.PHONY: sloccount_m
sloccount_m:
	$(call sloccount_doc_creation,$(SLOCCOUNT_SRCS))

.PHONY: help
help:
	$(SILENT)$(ECHO) 'Usage: make [target] [option]'
	$(SILENT)$(ECHO) 'target:'
	$(SILENT)$(ECHO) '  all                Build all targets (default)'
	$(SILENT)$(ECHO) '  release            Build all targets and create rpm, metrics'
	$(SILENT)$(ECHO) '  clean              Removes object files'
	$(SILENT)$(ECHO) '  distclean          Removes object and binary files'	
	$(SILENT)$(ECHO) '  help               Print help'
	$(SILENT)$(ECHO) 'option:'
	$(SILENT)$(ECHO) '  BUILD_VERBOSE=0|1  (0: quiet build (default), 1: verbose build)'
	$(SILENT)$(ECHO) '  COVERAGE=0|1       (0: no coverage (default), 1: coverage)'

