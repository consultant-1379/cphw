# **********************************************************************
#
# Short description:
# Makefile for gprshmadm_caa
# **********************************************************************
#
# Ericsson AB 2014-2017. All rights reserved.
# The information in this document is the property of Ericsson.
# Except as specifically authorized in writing by Ericsson, the receiver of this
# document shall keep the information contained herein confidential and shall protect
# the same in whole or in part from disclosure and dissemination to third parties.
# Disclosure and disseminations to the receivers employees shall only be made
# on a strict need to know basis.
#
# **********************************************************************
#
# Rev        Date	  Name      What
# -----      ---------    --------  --------------------------
#	     2014-12-11   xtuanguu  Created
#	     2016-12-10   xdtkebo   Adjusted for new product name
#       2017-02-08   xdargas   Changes for passivating service and events
#
# ********************************************************************

CNZ_NAME ?= gprshm_cnz
CAA_NAME ?= gprshmadm_caa
CXC_NAME ?= gprshm_cxc

# Basic paths
VOB_PATH = $(CPHW_ROOT)/GPRSHM
CNZ_PATH = $(VOB_PATH)/$(CNZ_NAME)
CAA_PATH = $(CNZ_PATH)/$(CAA_NAME)
CXC_PATH = $(CNZ_PATH)/$(CXC_NAME)
                                  
include $(COMMON_ROOT)/common.mk
                                 
DOXYGENFILECONFIG := $(COMMON_ROOT)/doxygen_common.conf


CQS_TOP = $(AFS_CP)/ThirdPartyProducts/GroupCommunicationMiddleware
RES_DIR = $(CQS_TOP)/lib/Product/gcc4/linux_x86_64

FTM_INCDIR = $(CQS_TOP)/src
FTM_LIBDIR = $(CQS_TOP)/lib/Product/gcc4/linux_x86_64/native

FTM_LIB += $(FTM_LIBDIR)/libnativevniftm.a
FTM_LIB += $(FTM_LIBDIR)/libnativevnishadow.a
FTM_LIB += $(FTM_LIBDIR)/libnativevnievolve.a
FTM_LIB += $(FTM_LIBDIR)/libnativevni.a

CXC_PATH = ../gprshm_cxc

# define objs want to build
OBJS = 	$(OBJDIR)/CPHW_GPRSHM_Main.o \
	$(OBJDIR)/CPHW_GPRSHM_Trace.o \
	$(OBJDIR)/CPHW_GPRSHM_Server.o \
	$(OBJDIR)/CPHW_GPRSHM_Service.o \
	$(OBJDIR)/CPHW_GPRSHM_FTmMedia.o \
	$(OBJDIR)/CPHW_GPRSHM_AP2CPPH.o \
	$(OBJDIR)/CPHW_GPRSHM_GPRFile.o \
	$(OBJDIR)/CPHW_GPRSHM_Event.o \
	$(OBJDIR)/vShlfGPRMsg.o \
	$(OBJDIR)/FTmMedia.o

CINCLUDES += -I$(INCDIR) -I$(FTM_INCDIR)

## # here you can add own Include paths and/or other INCDIRludes
#CINCLUDES += -I$(AP_SDK_INC) -I$(BOOST_SDK_INC) -I$(3PP_SDK_PATH)
CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(LOTC_SDK_INC) -I$(AP_SDK_INC) -I$(COREMW_SDK_INC) -I$(ACE_SDK_INC)

## # here you can add your own lib dirs
##LIBSDIR += -L$(AP_SDK_LIB) -L$(BOOST_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)
LIBSDIR  = -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LOTC_SDK_LIB) -L$(COREMW_SDK_LIB) -L$(ACE_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)
## # here you can add own libs
## TODO: remove this line & unmark the above line when need to use IMM & ACE stuff
LIBS += -lacs_apgcc -lacs_tra -llog4cplus  -lboost_thread -lboost_system -lpthread -ldl -lcrypto -lacs_csapi -lacs_aeh

#CFLAGS += -O3 -g -Wall -ansi -fno-strict-aliasing -DDEBUG -DCONSOLE_LOG
CFLAGS += -O0 -g -Wall -ansi -fno-strict-aliasing -DDEBUG

VPATH += $(SRCDIR) $(INCDIR) $(OUTDIR) $(OBJDIR)

GPRSHM_APNAME = cphw_gprshmd
GPRSHM_APEXE = $(OUTDIR)/$(GPRSHM_APNAME)

.PHONY: all
all: $(GPRSHM_APNAME)

.PHONY: $(GPRSHM_APNAME)
$(GPRSHM_APNAME): $(OBJS)
	$(SILENT)$(ECHO) 'Linking $@...'
	$(CC) -o $(GPRSHM_APEXE) $(OBJS) $(LIBSDIR) $(LIBS) $(FTM_LIB)
	$(call stripp,$(GPRSHM_APNAME))
	$(NEW_LINE)

# Include dependecies in order to build when header file changes
DEPS = $(OBJS:.o=.d)
ifeq ($(MAKECMDGOALS),all)
	-include $(DEPS)
else
	ifeq ($(MAKECMDGOALS),)
		-include $(DEPS)
	endif
endif

.PHONY: clean
clean:
	$(SILENT)$(RM) -r $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(GPRSHM_APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(GPRSHM_APNAME)_dbg

