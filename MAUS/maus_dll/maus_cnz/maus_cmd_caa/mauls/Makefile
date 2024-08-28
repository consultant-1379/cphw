# **********************************************************************
#
# Short description:
# Makefile maus
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

# Rev		 Date		  Name      What
# -----      ---------    --------  --------------------------
#		     2014-05-26   xdtthng   Created
#
# ********************************************************************

# The following variables are defined in common.mk file
#SRCDIR = src
#OBJDIR = obj
#OUTDIR = $(CXCDIR)/bin
#INCDIR = inc

CURDIR = $(shell pwd)
REPO_NAME = cphw
CPHW_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(CPHW_ROOT)/common
 
include $(COMMON_ROOT)/common.mk

CXCDIR = ../../maus1_cxc

APNAME = mauls
APEXE  = $(OUTDIR)/$(APNAME)

LOCK_APNAME = maulock
LOCK_APEXE = $(OUTDIR)/$(LOCK_APNAME)

UNLOCK_APNAME = mauunlock
UNLOCK_APEXE = $(OUTDIR)/$(UNLOCK_APNAME)

ALS_APNAME = mauadmls
ALS_APEXE = $(OUTDIR)/$(ALS_APNAME)

##MAUSET_APNAME = mauset
##MAUSET_APEXE = $(OUTDIR)/$(MAUSET_APNAME)

##MAUADM_APNAME = mauadm
##MAUADM_APEXE = $(OUTDIR)/$(MAUADM_APNAME)

# Define COMMON  paths
COMMON_DIR     = ../../maus_adm_caa/common
COMMON_INCDIR  = $(COMMON_DIR)/inc
COMMON_OBJDIR  = $(COMMON_DIR)/obj
COMMON_LIB     = $(COMMON_OBJDIR)/libcommon.a

# Define objs want to build
OBJ = $(OBJDIR)/mauls.o
LOCK_OBJ = $(OBJDIR)/maulock.o
UNLOCK_OBJ = $(OBJDIR)/mauunlock.o
ALS_OBJ = $(OBJDIR)/mauadmls.o
##MAUSET_OBJ = $(OBJDIR)/mauset.o
##MAUADM_OBJ = $(OBJDIR)/mauadm.o

##ALL_NAMES = $(OUTDIR)/$(APNAME) $(OUTDIR)/$(LOCK_APNAME) $(OUTDIR)/$(UNLOCK_APNAME) $(OUTDIR)/$(ALS_APNAME)
##ALL_NAMES += $(OUTDIR)/$(MAUSET_APNAME)

##ALL_NAMES = $(OUTDIR)/$(APNAME)
ALL_NAMES = $(OUTDIR)/$(APNAME) $(OUTDIR)/$(LOCK_APNAME) $(OUTDIR)/$(UNLOCK_APNAME) $(OUTDIR)/$(ALS_APNAME)

CINCLUDES += -I$(INCDIR) -I$(COMMON_INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(ACE_SDK_INC)
LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB) -L$(COREMW_SDK_LIB)
LIBS += -lboost_regex -lboost_system -lboost_thread -lacs_tra -lSaImmOm
LIBS += -lacs_csapi -lacs_prc -lacs_dsd -lacs_apgcc -lcrypto

# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif

#CFLAGS += -g -Wall -ansi -DNDEBUG -DLOCAL_BUILD
CFLAGS += -g -Wall -ansi 

LDFLAGS += -Wl,-rpath-link,$(AP_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(COREMW_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_CREDU_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_XERCES_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(BOOST_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(ACE_SDK_LIB)

.PHONY: all
all: $(ALL_NAMES)
	@echo "MAUS command  output files are created "

.PHONY: common
common:
# These command will be executed in another shell (not the current shell)
	cd $(COMMON_DIR); $(MAKE) $(MFLAGS) all

   
$(OUTDIR)/$(APNAME): $(OBJ) common
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(APEXE) $(OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(COMMON_LIB)
	$(SILENT)$(ECHO) '$(APEXE) is created'
	$(call stripp,$(APNAME))
	$(NEW_LINE)	

##$(OUTDIR)/$(MAUADM_APNAME): $(MAUADM_OBJ) common
##	$(SILENT)$(ECHO) 'Linking $@... '
##	$(SILENT)$(CC) -o $(MAUADM_APEXE) $(MAUADM_OBJ) $(LIBSDIR) $(LIBS) $(COMMON_LIB)
##	$(SILENT)$(ECHO) '$(MAUADM_APEXE) is created'
##	$(NEW_LINE)	

##$(OUTDIR)/$(MAUSET_APNAME): $(MAUSET_OBJ) common
##	$(SILENT)$(ECHO) 'Linking $@... '
##	$(SILENT)$(CC) -o $(MAUSET_APEXE) $(MAUSET_OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(COMMON_LIB)
##	$(SILENT)$(ECHO) '$(MAUSET_APEXE) is created'
##	$(NEW_LINE)	

$(OUTDIR)/$(LOCK_APNAME): $(LOCK_OBJ) common
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(LOCK_APEXE) $(LOCK_OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(COMMON_LIB)
	$(SILENT)$(ECHO) '$(LOCK_APEXE) is created'
	$(call stripp,$(LOCK_APNAME))
	$(NEW_LINE)	

$(OUTDIR)/$(UNLOCK_APNAME): $(UNLOCK_OBJ) common
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(UNLOCK_APEXE) $(UNLOCK_OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(COMMON_LIB)
	$(SILENT)$(ECHO) '$(UNLOCK_APEXE) is created'
	$(call stripp,$(UNLOCK_APNAME))
	$(NEW_LINE)	

$(OUTDIR)/$(ALS_APNAME): $(ALS_OBJ) common
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(ALS_APEXE) $(ALS_OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(COMMON_LIB)
	$(SILENT)$(ECHO) '$(ALS_APEXE) is created'
	$(call stripp,$(ALS_APNAME))
	$(NEW_LINE)

# Include dependecies in order to build when header file changes
DEPS = $(OBJ:.o=.d)
ifeq ($(MAKECMDGOALS),all)	
	-include $(DEPS)
else
	ifeq ($(MAKECMDGOALS),)
		-include $(DEPS)
	endif
endif

# this rule is defined in common.mk
#$(OBJDIR)/%.o: $(SRCDIR)/%.cpp

.PHONY: clean
clean:
	cd $(COMMON_DIR); $(MAKE) $(MFLAGS) clean;
	$(SILENT)$(RM) -r $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(APNAME)_dbg
	$(SILENT)$(RM) -r $(OUTDIR)/$(LOCK_APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(LOCK_APNAME)_dbg
	$(SILENT)$(RM) -r $(OUTDIR)/$(UNLOCK_APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(UNLOCK_APNAME)_dbg
	$(SILENT)$(RM) -r $(OUTDIR)/$(ALS_APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(ALS_APNAME)_dbg

