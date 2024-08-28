# **********************************************************************
#
# Short description:
# Makefile for syscon_common
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
#		     2015-10-22   xdtthng   Created
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

# Define objs want to build
COMMON_OBJ = $(OBJDIR)/AppTrace.o \
			$(OBJDIR)/CmdFunx.o \
			$(OBJDIR)/DmxGepInfo.o \
			$(OBJDIR)/FileFunx.o \
			$(OBJDIR)/GepFunx.o \
			$(OBJDIR)/GepInfo.o \
			$(OBJDIR)/ParseIni.o \
			$(OBJDIR)/SysFunx.o \
			$(OBJDIR)/ScbGepInfo.o \
			$(OBJDIR)/ScxGepInfo.o \
			$(OBJDIR)/SmxGepInfo.o \
			$(OBJDIR)/ScoPinger_30.o

# Define own Include paths
CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH)  -I$(ACE_SDK_INC)

# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif

CFLAGS += -g -Wall -ansi -std=c++11
 

# Define TARGET
TARGET = libsysconcommon.a

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

