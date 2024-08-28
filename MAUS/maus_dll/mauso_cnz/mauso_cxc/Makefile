################################################################################
#                                                                              #
# Name: Makefile                                                               #
#                                                                              #
#	 COPYRIGHT (C) Ericsson AB 2010-2022                                   #
#                                                                              #
#	 The program(s) may be used and/or copied only with the written        #
#	 permission from Ericsson AB or in accordance with the                 #
#	 terms and conditions stipulated in the agreement/contract under       #
#	 which the program(s) have been supplied.                              #
#                                                                              #
#                                                                              #
################################################################################
#
# The file is readable for both make, gmake and clearmake.
#
# Responsible :  EAB/FLE/D
# Date		  :  2010-11-08
#

################################################################################
#		Definition Section
################################################################################
#
# MAUSOBIN
# CAA_NR = CAA1091420
# BEGIN: GIT INTRO.
CURDIR = $(shell pwd)
REPO_NAME = cphw
CPHW_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(CPHW_ROOT)/common

# common makefile
include $(COMMON_ROOT)/common.mk 
 
CXC_NR ?= CXC1372148_16
CXC_OLD_VER = ''
CXC_VER ?= R1E
CXC_NAME ?= CPHW_MAUSOBIN
BLOCK_NAME = $(CXC_NAME)
BASE_SW_VER = 3.8.0-R1B
CXC_BLK_NAME ?= cphw_mauso
FX_CLEAN_TMP := rm -rf /tmp/$(USER)/*
#ESM_VER = $(shell $(ESM_SCRIPTS_DIR)/getversion.sh $(CXC_NR) $(CXC_VER))

CXCPATH=$(CURDIR)
CXCDIR=$(CXCPATH)

# Name directories to iterate over
# --------------------------------

CAA_BUILD_DIRS = \
	bin/lib_ext

# test
DIRS = $(CAA_DIRS)

#
#	Set the make top directory
ifeq ($(MAKE_TOP),)
  MAKE_TOP = $(PWD)
endif

################################################################################
#										 Rule Section
################################################################################
#
# Declaration of phony targets i.e. a phony target will always be invoked
# regardless whether it exists a file or directory with the same name as
# the 'target'.
#

#.PHONY: $(DIRS)
#
# Iterate over all subdirectories listed
# in the DIRS macro and invoke Make on the directory.
#
#$(DIRS) :
#	@if [ -d $@ ]; then\
#		if [ -r [Mm]akefile ]; then\
#		  cd $@;\
#		  $(MAKE) CONFIG=$(CONFIG) MAKE_TOP=$(MAKE_TOP) -f [Mm]akefile $(MAKECMDGOALS);\
#		fi;\
#	 fi

#
# Iterate over all subdirectories listed
# in the CAA_DIRS macro and invoke Make on the directory.
#
.PHONY: all
all:
	$(ECHO) "please see the printout !!!"
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
	@for i in $(CAA_DIRS); do \
	 (cd $$i;\
		$(MAKE) $(MFLAGS) COMMON_ROOT=$(COMMON_ROOT) CPHW_ROOT=$(CPHW_ROOT) clean);\
	done
	$(call FX_CLEAN_TMP)
.PHONY: distclean
distclean: 
	rm -rf bin/lib_ext/*;rm -rf packages/rpm/*.rpm;rm -rf packages/sdp/*;
	$(call FX_CLEAN_TMP)

.PHONY: rpm_package
rpm_package: 
	$(CXCPATH)/collect_mau_core_libs.sh $(CXC_NR)_$(CXC_VER) $(CXCPATH)
	$(call rpm_file_creation, MAUSOBIN.spec,$(CXC_NAME),$(CXC_NR),$(CXC_VER))
	
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

################################################################################
#										  End of Makefile											 #
################################################################################

