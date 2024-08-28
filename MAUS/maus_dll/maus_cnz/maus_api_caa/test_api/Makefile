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

APNAME = maulib
APEXE  = $(OUTDIR)/$(APNAME)

# Define COMMON  paths
COMMON_DIR     = ../../maus_adm_caa/common
COMMON_INCDIR  = $(COMMON_DIR)/inc
COMMON_OBJDIR  = $(COMMON_DIR)/obj
COMMON_LIB     = $(COMMON_OBJDIR)/libcommon.a

MAUS_LIBH_DIR = $(CXCDIR)/bin/lib_ext
MAUS_LIBH_INCDIR = ../inc_ext

# Define objs want to build
OBJ = $(OBJDIR)/$(APNAME).o

CINCLUDES += -I$(INCDIR) -I$(COMMON_INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC)
CINCLUDES += -I$(MAUS_LIBH_INCDIR)

LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L$(MAUS_LIBH_DIR)

LIBS += -lboost_regex -lboost_system -lboost_thread -lacs_tra -lacs_csapi
LIBS += -lcphw_mauslibh

# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif

#CFLAGS += -g -Wall -ansi -DNDEBUG -DLOCAL_BUILD
CFLAGS += -g -Wall -ansi 

.PHONY: all
all: $(OUTDIR)/$(APNAME)

.PHONY: common
common:
# These command will be executed in another shell (not the current shell)
	cd $(COMMON_DIR); $(MAKE) $(MFLAGS) all

   
$(OUTDIR)/$(APNAME): $(OBJ) common
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(APEXE) $(OBJ) $(LIBSDIR) $(LIBS) $(COMMON_LIB)
	$(SILENT)$(ECHO) '$(APEXE) is created'
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

