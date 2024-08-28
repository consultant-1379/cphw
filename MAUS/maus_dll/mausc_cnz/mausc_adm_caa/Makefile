# **********************************************************************
#
# Short description:
# Makefile mausc
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
#		     2015-02-10   xdtthng   Created
#
# ********************************************************************

# The following variables are defined in common.mk file
#SRCDIR = src
#OBJDIR = obj
#OUTDIR = $(CXC_PATH)/bin
#INCDIR = inc

# CS Black delivery
### BLACKDIR = ../maus_black
### BLACK_INC		= $(BLACKDIR)/inc
### BLACK_LIB		= $(BLACKDIR)/lib

NTACS_PATH=../../../../../../../cm4ap/ntacs
CNZ_NAME ?= mausc_cnz
CAA_NAME ?= mausc_adm_caa
CXC_NAME ?= mausc_cxc

# Basic paths
VOB_PATH = $(CPHW_ROOT)/MAUS/maus_dll
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)

CXCDIR = $(CXC_PATH)

MAUSC_APNAME = cphw_mauscd
MAUSC_APEXE  = $(OUTDIR)/$(MAUSC_APNAME)
                                  
include $(COMMON_ROOT)/common.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf

#include /vobs/IO_Developments/Tools/linux_conf_file/common.mk

# Define objs want to build
MAUSC_OBJ = $(OBJDIR)/main_mausc.o \
			$(OBJDIR)/CpMauData.o \
			$(OBJDIR)/CpMauM.o \
			$(OBJDIR)/CpMauMsg.o \
			$(OBJDIR)/CptSwMau.o \
			$(OBJDIR)/CptSwMauMsg.o \
			$(OBJDIR)/FileFunx.o \
			$(OBJDIR)/GMClient.o \
			$(OBJDIR)/MAUFunx.o \
			$(OBJDIR)/MauMsg.o \
			$(OBJDIR)/MausCpTableObserver.o \
			$(OBJDIR)/Maus_AehEvent.o \
			$(OBJDIR)/MauscService.o \
			$(OBJDIR)/MausSysInfo.o \
			$(OBJDIR)/MAServer.o \
			$(OBJDIR)/ParseIni.o \
			$(OBJDIR)/SwMauMom.o \
			$(OBJDIR)/SwMauOI.o \
			$(OBJDIR)/SwMauOM.o \
			$(OBJDIR)/TCPAcceptor.o \
			$(OBJDIR)/TCPConnection.o \
			$(OBJDIR)/TCPServer.o \
			$(OBJDIR)/maus_trace.o

##$(OBJDIR)/MausOM.o \
##$(OBJDIR)/MausOI.o \
##$(OBJDIR)/MausModel.o \
      
# Define own Include paths
### CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH)
CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH)
CINCLUDES += -I$(COREMW_SDK_INC) -I$(ACE_SDK_INC)

# Define own lib paths
### LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)

# Define own Libs
LIBS += -lboost_filesystem -lboost_regex -lboost_system -lboost_thread -lacs_apgcc -lacs_tra 
LIBS += -lacs_csapi -lacs_aeh -lcap -lacs_dsd -lacs_prc 
# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif

##CFLAGS += -g -Wall -ansi -std=c++11
CFLAGS += -g -Wall -ansi

LDFLAGS += -Wl,-rpath-link,$(AP_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(COREMW_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_CREDU_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_XERCES_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(BOOST_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(ACE_SDK_LIB)

.PHONY: all
all: $(OUTDIR)/$(MAUSC_APNAME)
   
$(OUTDIR)/$(MAUSC_APNAME): $(MAUSC_OBJ) 
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(MAUSC_APEXE) $(MAUSC_OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(CC) --version
	$(SILENT)$(ECHO) '$(MAUSC_APEXE) is created'
	$(call stripp,$(MAUSC_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)	

.PHONY: clean
clean:
	$(SILENT)$(RM) -r $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(MAUSC_APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(MAUSC_APNAME)_dbg

# Include dependecies in order to build when header file changes
DEPS = $(BUSRV_OBJ:.o=.d)
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
	$(SILENT)$(RM) -r $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(MAUSC_APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(MAUSC_APNAME)_dbg

