# **********************************************************************
#
# Short description:
# Makefile for maus1_cxc
# **********************************************************************
#
# Ericsson AB 2014 All rights reserved.
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
#            2011-01-19   xdtthng   Created
#
# **********************************************************************
 
# BEGIN: GIT INTRO.
CURDIR = $(shell pwd)
REPO_NAME = cphw
CPHW_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(CPHW_ROOT)/common

include $(COMMON_ROOT)/common.mk 

CXC_NAME    ?= CPHW_MAUSLBIN
BLOCK_NAME = $(CXC_NAME)
CXC_NR      ?= CXC1372174_9
CXC_VER     ?= R1C
CXC_OLD_VER ?= R1A01

BASE_SW_VER ?= 3.6.0-R1A

CXC_BLK_NAME ?= cphw_mausl

CURDIR  = $(shell pwd)
CXCPATH = $(CURDIR)
CXCDIR  = $(CXCPATH)

CAA_BUILD_DIRS = \
	bin/lib_ext

#ESM_VER = $(shell $(ESM_SCRIPTS_DIR)/getversion.sh $(CXC_NR) $(CXC_VER))
FX_CLEAN_TMP := rm -rf /tmp/$(USER)/*

# test
DIRS = $(CAA_DIRS)

#
#	Set the make top directory
ifeq ($(MAKE_TOP),)
  MAKE_TOP = $(PWD)
endif

.PHONY: all
all:
$(ECHO) "Error: please see the printout !!!"
#	@for i in $(CAA_DIRS); do \
#	 (cd $$i;\
#		$(MAKE) $(MFLAGS) CXCDIR=$(CXCPATH) all);\
#	 if [ $$? != 0 ] ; \
#	  then $(ECHO) "Error: please see the printout !!!";\
#	  exit 3;\
#	  fi;\
#	done

.PHONY: del_update
del_update:
	$(call rpm_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call sdp_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_CXC_VER),$(CXC_NR))
	$(call esm_del_update,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(OLD_ESM_VER),$(CXC_NR))

.PHONY: clean
clean: 
#	@for i in $(CAA_DIRS); do \
#	 (cd $$i;\
#		$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) CPHW_ROOT=$(CPHW_ROOT) clean);\
#	done
	
#.PHONY: distclean
#distclean: 
#	rm -rf bin/lib_ext/*;rm -rf packages/rpm/*.rpm;rm -rf packages/sdp/*;

.PHONY: rpm_package
rpm_package: 
	$(call rpm_file_creation, mausl.spec,$(CXC_NAME),$(CXC_NR),$(CXC_VER))
	
.PHONY: sdp_package
sdp_package:
	$(SILENT)$(ECHO) 'SDP packaging ...' \
	$(call sdp_creation_US2,$(CXCPATH),$(BLOCK_NAME),$(CXC_VER),$(CXC_OLD_VER),$(CXC_NR))
	$(call up_creation,$(CXC_NAME),$(CXC_NR),$(CXC_VER),$(CXC_OLD_VER),$(CXCPATH),$(BASE_SW_VER))	

.PHONY: esm_package
esm_package:
	$(call esm_pkg_creation,$(CXCPATH),$(CXC_BLK_NAME),$(CXC_VER),$(BASE_SW_VER),$(CXC_NR),$(CXC_NAME))
                      
.PHONY: upload_package 
upload_package: 
	$(call upload_package,$(CXC_BLK_NAME),$(CXC_NR),$(CXC_NAME),$(CXC_VER))

.PHONY: tar_package
tar_package:
	$(SILENT)$(ECHO) 'TAR packaging ...'
	$(SILENT)$(ECHO) '**********************************'
	$(SILENT)$(ECHO) '****** NOT YET IMPLEMENTED *******'
	$(SILENT)$(ECHO) '**********************************'

