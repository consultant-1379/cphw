# **********************************************************************
#
# Short description:
# Makefile TEST
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
#OUTDIR = $(CXCDIR)/bin
#INCDIR = inc

CXCDIR ?= ../mausc_cxc

TEST_APNAME = maum
TEST_APEXE  = $(OUTDIR)/$(TEST_APNAME)

MAUTYPE_APNAME = mautype
MAUTYPE_APEXE = $(OUTDIR)/$(MAUTYPE_APNAME)

MCLIENT_APNAME = fexclient
MCLIENT_APEXE  = $(OUTDIR)/$(MCLIENT_APNAME)

# CS Black delivery
##BLACKDIR = ../maus_black
##BLACK_INC		= $(BLACKDIR)/inc
##BLACK_LIB		= $(BLACKDIR)/lib

CURDIR = $(shell pwd)
REPO_NAME = cphw
CPHW_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(CPHW_ROOT)/common
 
include $(COMMON_ROOT)/common.mk

# Define objs want to build
TEST_OBJ = $(OBJDIR)/main_maum.o \
			$(OBJDIR)/CpMauM.o \
			$(OBJDIR)/GMClient.o \
			$(OBJDIR)/maus_trace.o \
			$(OBJDIR)/MauMsg.o

MAUTYPE_OBJ = $(OBJDIR)/main_mautype.o	
MCLIENT_OBJ = $(OBJDIR)/main_client.o $(OBJDIR)/CpMauMsg.o 	
      
# Define own Include paths
##CINCLUDES += -I$(INCDIR) -I$(BLACK_INC) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH)
CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH)

# Define own lib paths
##LIBSDIR += -L$(BOOST_SDK_LIB) -L$(BLACK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)

# Define own Libs
LIBS += -lboost_regex -lboost_system -lboost_thread -lacs_apgcc -lacs_tra -lacs_csapi -lacs_aeh -lcap

# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif

CFLAGS += -g -Wall -ansi 

.PHONY: all
all: $(OUTDIR)/$(TEST_APNAME) $(OUTDIR)/$(MAUTYPE_APNAME) $(OUTDIR)/$(MCLIENT_APNAME)
   
$(OUTDIR)/$(TEST_APNAME): $(TEST_OBJ) 
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(TEST_APEXE) $(TEST_OBJ) $(LIBSDIR) $(LIBS)
	$(SILENT)$(ECHO) '$(TEST_APEXE) is created'
	$(call stripp,$(TEST_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)	

$(OUTDIR)/$(MAUTYPE_APNAME): $(MAUTYPE_OBJ) 
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(MAUTYPE_APEXE) $(MAUTYPE_OBJ) $(LIBSDIR) $(LIBS)
	$(SILENT)$(ECHO) '$(MAUTYPE_APEXE) is created'
	$(call stripp,$(MAUTYPE_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)	

$(OUTDIR)/$(MCLIENT_APNAME): $(MCLIENT_OBJ) 
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(MCLIENT_APEXE) $(MCLIENT_OBJ) $(LIBSDIR) $(LIBS)
	$(SILENT)$(ECHO) '$(MCLIENT_APEXE) is created'
	$(call stripp,$(MCLIENT_APNAME))
	$(SEPARATOR_STR)
	$(NEW_LINE)	

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
	$(SILENT)$(RM) -r $(OUTDIR)/$(TEST_APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(TEST_APNAME)_dbg

