# **********************************************************************
#
# Short description:
# Makefile syscon_adm_caa
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
# Rev		 Date		  Name      What
# -----      ---------    --------  --------------------------
#		     2015-10-21   xdtthng   Created
#                    2017-09-29   teisdel   updated for SDK_3_5
#
# ********************************************************************

# The following variables are defined in common.mk file
#SRCDIR = src
#OBJDIR = obj
#OUTDIR = $(CXC_PATH)/bin
#INCDIR = inc

# Define paths to block internal staric libraries
SYSCON_COMMON_DIR     = ../syscon_common
SYSCON_COMMON_INCDIR  = $(SYSCON_COMMON_DIR)/inc
SYSCON_COMMON_OBJDIR  = $(SYSCON_COMMON_DIR)/obj
SYSCON_COMMON_LIB     = $(SYSCON_COMMON_OBJDIR)/libsysconcommon.a

# OpenIPMI lib dir
OPEN_IPMI_LIBDIR = ../openipmi/lib64
OPEN_IPMI_LIBINC = ../openipmi/include
NTACS_PATH=../../../../../../../cm4ap/ntacs
CNZ_NAME ?= syscon_cnz
CAA_NAME ?= syscon_cmd_caa
CXC_NAME ?= syscon_cxc

# Basic paths
VOB_PATH = $(CPHW_ROOT)/MAUS/maus_dll
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)

CXCDIR = $(CXC_PATH)

SYSCON_APNAME = syscon
SYSCON_APEXE  = $(OUTDIR)/$(SYSCON_APNAME)

SYSLOGLS_APNAME = syslogls
SYSLOGLS_APEXE  = $(OUTDIR)/$(SYSLOGLS_APNAME)
                                  
include $(COMMON_ROOT)/common.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

# Define SYSCON objs
SYSCON_OBJ = $(OBJDIR)/cmd_syscon_main_40.o \
				$(OBJDIR)/CmdError.o \
				$(OBJDIR)/IPAddressTable.o \
				$(OBJDIR)/Keyboard.o \
				$(OBJDIR)/SolConnect.o \
				$(OBJDIR)/SolSession.o \
				$(OBJDIR)/SysconClient.o
												
# here you can add own Include paths and/or other includes before those included by common.mk file
#-I/afs/sunrise.ericsson.se/eu/app/APG43L/SDK/3pp/BOOST/boost_1_54_0/include
CINCLUDES += -I$(INCDIR)
CINCLUDES += -I$(SYSCON_COMMON_INCDIR)
CINCLUDES += -I$(AP_SDK_INC)
CINCLUDES += -I$(3PP_SDK_PATH)
CINCLUDES += -I$(BOOST_SDK_INC)
CINCLUDES += -I$(COREMW_SDK_INC)
CINCLUDES += -I$(ACE_SDK_INC)
CINCLUDES += -I$(LOTC_SDK_INC)
CINCLUDES += -I$(OPEN_IPMI_LIBINC)

## here you can add own lib paths
LIBSDIR += -L$(LIB_EXT)
LIBSDIR += -L$(AP_SDK_LIB)
LIBSDIR += -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L$(BOOST_SDK_LIB)
LIBSDIR += -L$(LOTC_SDK_LIB)
LIBSDIR += -L$(ACE_SDK_LIB)
LIBSDIR += -L$(COREMW_SDK_LIB)
LIBSDIR += -L$(OPEN_IPMI_LIBDIR)

# here you can add own libs
LIBS += -lrt -lpthread -lACE
LIBS += -lacs_tra -lcap 
LIBS += -lacs_aeh -lacs_apgcc -lacs_csapi -lacs_prc -lacs_alog  # -llog4cplus
LIBS += -lboost_system -lboost_regex -lboost_thread
LIBS += -lOpenIPMI -lOpenIPMIutils -lOpenIPMIposix -lOpenIPMIpthread 

# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif

CFLAGS += -g -Wall -ansi -std=c++11

.PHONY: all
all: $(OUTDIR)/$(SYSCON_APNAME)

.PHONY: sysconcommon
sysconcommon:
# These command will be executed in another shell (not the current shell)
	cd $(SYSCON_COMMON_DIR); $(MAKE) $(MFLAGS) all

$(OUTDIR)/$(SYSCON_APNAME): $(SYSCON_OBJ) sysconcommon
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(SYSCON_APEXE) $(SYSCON_OBJ) $(SYSCON_COMMON_LIB) $(LIBSDIR) $(LIBS)
	$(SILENT)$(ECHO) '$(SYSCON_APEXE) is created'
	$(call stripp,$(SYSCON_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)	

# Include dependecies in order to build when header file changes
DEPS = $(SYSCON_OBJ:.o=.d)
ifeq ($(MAKECMDGOALS),all)	
	-include $(DEPS)
else
	ifeq ($(MAKECMDGOALS),)
		-include $(DEPS)
	endif
endif

.PHONY: clean
clean:
	cd $(SYSCON_COMMON_DIR); $(MAKE) $(MFLAGS) clean;
	$(SILENT)$(RM) -r $(OBJDIR)/*

.PHONY: distclean
distclean: clean
		$(SILENT)$(RM) $(OUTDIR)/$(SYSCON_APNAME)
		

