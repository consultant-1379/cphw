# **********************************************************************
#
# Short description:
# Makefile for CPHW Lih library
# **********************************************************************
#
# Ericsson AB 2015 All rights reserved.
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
# -----      ---------    --------  --------------------------
#            2015-05-05   xdtthng   Created
#
# ********************************************************************

# **********************************************************************
#
#
# ********************************************************************

#the following variables are defined in common.mk file
#BLOCKDIR = $(CURDIR)
#OBJDIR   = $(BLOCKDIR)/obj
#SRCDIR   = $(BLOCKDIR)/src
#INCDIR   = $(BLOCKDIR)/inc
#TESTDIR  = $(BLOCKDIR)/test
#OUTDIR   = $(CXC_PATH)/bin

CXC_PATH = ../maus1_cxc
MAUSL_DIR = ../mausl_cxc

# Define lib variables
LIB_NAME = cphw_mauslibh
LIB_REL_MAJOR ?= 1
LIB_REL_MINOR ?= 1
LIB_REL_BUILD ?= 2
LIB_VER ?= $(LIB_REL_MAJOR).$(LIB_REL_MINOR).$(LIB_REL_BUILD)
LIB_FILENAME = lib$(LIB_NAME).so.$(LIB_VER)
LIB_SONAME = lib$(LIB_NAME).so.$(LIB_REL_MAJOR)
LIB_LINKER_NAME = lib$(LIB_NAME).so

MAUS_LIH_APLIB = $(OUTDIR)/$(LIB_FILENAME)
MAUS_LIH_APLIB_SO = $(OUTDIR)/$(LIB_LINKER_NAME)

# Define COMMON  paths
COMMON_DIR     = ../maus_adm_caa/common
COMMON_INCDIR  = $(COMMON_DIR)/inc
COMMON_OBJDIR  = $(COMMON_DIR)/obj
COMMON_LIB     = $(COMMON_OBJDIR)/libcommon.a

# Define MAUS  paths
MAUS_DIR     = ../maus_adm_caa/maus
MAUS_INCDIR  = $(MAUS_DIR)/inc
MAUS_OBJDIR  = $(MAUS_DIR)/obj
MAUS_LIB     = $(MAUS_OBJDIR)/maus.a

LIBH_DIR     	= ./
LIBH_INCDIR	= $(LIBH_DIR)/inc_ext


CNZ_NAME ?= maus_cnz
CAA_NAME ?= maus_api_caa
CXC_NAME ?= maus_cxc

# Basic paths
VOB_PATH = $(CPHW_ROOT)/MAUS/maus_dll
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(COMMON_ROOT)/common.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

# If modified, must be placed after the above line
OUTDIR   = $(MAUSL_DIR)/bin/lib_ext

# Define objs want to build
LIBHDEF_OBJ = $(OBJDIR)/CPHW_MAUS_API_Libh_R1.o \
	$(OBJDIR)/CPHW_MAUS_API_Libh_Impl.o
				      
# Define own Include paths
CINCLUDES += -I$(INCDIR) -I$(LIBH_INCDIR) -I$(COMMON_INCDIR) -I$(MAUS_INCDIR) \
-I$(COREMW_SDK_INC) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(ACE_SDK_INC)

# Define own lib paths
LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)

# Define own Libs 
#LIBS += -lboost_regex -lboost_system -lboost_thread -lacs_apgcc -llog4cplus -lacs_tra -lacs_csapi -lacs_aeh -lfms_cpf
LIBS += -lacs_dsd

# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif
#CFLAGS += -g -Wall -ansi -DNDEBUG -DLOCAL_BUILD
CFLAGS += -g -Wall -ansi -DNDEBUG 
LDFLAGS += -shared -Wl,-soname=$(LIB_SONAME)

.PHONY: all

all: $(OUTDIR)/$(LIB_FILENAME)

$(OUTDIR)/$(LIB_FILENAME): $(LIBHDEF_OBJ) maus common
	$(SILENT)$(ECHO) 'Creating library: $(OUTDIR)/$(LIB_FILENAME)'
	$(SILENT)$(CC) -o $(MAUS_LIH_APLIB) $(LIBHDEF_OBJ) $(COMMON_LIB) $(MAUS_LIB) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(call stripp,$(LIB_FILENAME))
	$(SILENT)$(ECHO) '$(OUTDIR)/$(LIB_FILENAME) is created'
	$(NEW_LINE)	

# Include dependecies in order to build when header file changes
DEPS = $(BUPDEF_OBJ:.o=.d)
ifeq ($(MAKECMDGOALS),all)	
	-include $(DEPS)
else
	ifeq ($(MAKECMDGOALS),)
		-include $(DEPS)
	endif
endif

.PHONY: common
common:
# These command will be executed in another shell (not the current shell)
	cd $(COMMON_DIR); $(MAKE) $(MFLAGS) all

.PHONY: maus
maus:
# These command will be executed in another shell (not the current shell)
	cd $(MAUS_DIR); $(MAKE) $(MFLAGS) all

# this rule is defined in common.mk
#$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
.PHONY: clean
clean:
	cd $(MAUS_DIR); $(MAKE) $(MFLAGS) clean;
	cd $(COMMON_DIR); $(MAKE) $(MFLAGS) clean;
	$(SILENT)$(RM) -r $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(LIB_FILENAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(LIB_FILENAME)_dbg

