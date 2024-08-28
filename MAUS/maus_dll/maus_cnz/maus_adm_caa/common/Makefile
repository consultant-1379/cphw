# **********************************************************************
#
# Short description:
# Makefile for common
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
#		     2014-05-24   xdtthng   Created
#
# ********************************************************************

CURDIR = $(shell pwd)
REPO_NAME = cphw
CPHW_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(CPHW_ROOT)/common
 
include $(COMMON_ROOT)/common.mk

# The following variables are defined in common.mk file
#SRCDIR = src
#OBJDIR = obj
#OUTDIR = $(CXCDIR)/bin
#INCDIR = inc

MAUS_DIR     = ../../maus_adm_caa/maus
MAUS_INCDIR  = $(MAUS_DIR)/inc
MAUS_OBJDIR  = $(MAUS_INCDIR)/obj

# CS Black delivery
## BLACKDIR = ../../maus_black
## BLACK_INC		= $(BLACKDIR)/inc
## BLACK_LIB		= $(BLACKDIR)/lib

# Define objs want to build
COMMON_OBJ = $(OBJDIR)/CmdClient.o \
		$(OBJDIR)/CmdErrMsg.o \
		$(OBJDIR)/CmdFunx.o \
		$(OBJDIR)/DSDServer.o \
		$(OBJDIR)/EndpointView.o \
		$(OBJDIR)/Exception.o \
		$(OBJDIR)/FileFunx.o \
		$(OBJDIR)/GMClient.o \
		$(OBJDIR)/GMServer.o \
		$(OBJDIR)/LibTrace.o \
		$(OBJDIR)/MAUConnection.o \
		$(OBJDIR)/MAUFunx.o \
		$(OBJDIR)/MAUMsgQ.o \
		$(OBJDIR)/MauMsg.o \
		$(OBJDIR)/Maus_AehEvent.o \
		$(OBJDIR)/Maus_ApConn.o \
		$(OBJDIR)/maus_sha.o \
		$(OBJDIR)/maus_trace.o \
		$(OBJDIR)/ParseIni.o \
		$(OBJDIR)/SwMauMom.o \
		$(OBJDIR)/TempDir.o \
		$(OBJDIR)/Text_String.o

#		$(OBJDIR)/sha256.o
#		$(OBJDIR)/Maus_CmdParam.o \
#		$(OBJDIR)/MausModel.o \
#		$(OBJDIR)/MAUApMsgQ.o \
#		$(OBJDIR)/BMsgQ.o \

# Define own Include paths
#CINCLUDES += -I$(INCDIR) -I$(BUPROT_INCDIR) -I$(BUSRV_INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH)
#CINCLUDES += -I$(INCDIR)  -I$(BLACK_INC) -I$(MAUS_INCDIR) -I$(MAUCORE_INCDIR) -I$(CPHW_INCDIR)
#CINCLUDES += -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(MAUCONN_INCDOR)

### CINCLUDES += -I$(INCDIR)  -I$(BLACK_INC) -I$(MAUS_INCDIR) 
CINCLUDES += -I$(INCDIR) -I$(MAUS_INCDIR) -I$(COREMW_SDK_INC) -I$(ACE_SDK_INC)
CINCLUDES += -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) 

#ifeq ($(USE_ACS_TRACE_DEBUG_FLAG),1)
#	CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC)
#else
#	CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) 
#endif

# Define own CFLAGS
#ifeq ($(COVERAGE),1)
#	CFLAGS += -O0
#else
#	CFLAGS += -Os
#endif

CFLAGS += -O

CFLAGS += -g -Wall -ansi -DNDEBUG

# Define TARGET
TARGET = libcommon.a

.PHONY: all
all: $(OBJDIR)/$(TARGET)

.PHONY: all
all: $(OBJDIR)/$(TARGET)

$(OBJDIR)/$(TARGET): $(COMMON_OBJ)
	$(SILENT)$(ECHO) 'Creating static library: $(TARGET)'
	$(SILENT)$(AR) -rcs $(OBJDIR)/$(TARGET) $(COMMON_OBJ) 
	$(NEW_LINE)

# Include dependecies in order to build when header file changes
DEPS = $(COMMON_OBJ:.o=.d)
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

