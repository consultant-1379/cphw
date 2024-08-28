# **********************************************************************
#
#
# ********************************************************************

CURDIR = $(shell pwd)
REPO_NAME = cphw
CPHW_ROOT = $(shell echo $(CURDIR) | sed 's@'/$(REPO_NAME)'.*@'/$(REPO_NAME)'@g')
COMMON_ROOT = $(CPHW_ROOT)/common
 
include $(COMMON_ROOT)/common.mk

#the following variables are defined in common.mk file
#BLOCKDIR = $(CURDIR)
#OBJDIR   = $(BLOCKDIR)/obj
#SRCDIR   = $(BLOCKDIR)/src
#INCDIR   = $(BLOCKDIR)/inc
#TESTDIR  = $(BLOCKDIR)/test
#OUTDIR   = $(CXCDIR)/bin

# CS Black delivery
## BLACKDIR = ../../maus_black
## BLACK_INC		= $(BLACKDIR)/inc
## BLACK_LIB		= $(BLACKDIR)/lib

# Define COMMON  paths
COMMON_DIR     = ../../maus_adm_caa/common
COMMON_INCDIR  = $(COMMON_DIR)/inc
COMMON_OBJDIR  = $(COMMON_DIR)/obj
COMMON_LIB     = $(COMMON_OBJDIR)/libcommon.a

# Define objs want to build
OBJ = $(OBJDIR)/ApMaus.o \
		$(OBJDIR)/Maus_Application.o\
		$(OBJDIR)/Maus_Service.o \
		$(OBJDIR)/Maus_SystemInformation.o

		###$(OBJDIR)//Maus_CommandServer.o\
		###$(OBJDIR)/Maus_TerminateHandler.o\
		###$(OBJDIR)/MausCoreSimulation.o \
		###$(OBJDIR)/Maus_ApConn.o \
		###$(OBJDIR)/Maus_ApHandler.o \
		###$(OBJDIR)/Maus_ApplicationManager.o\
		##$(OBJDIR)/Maus_SystemInformation.o		
		###$(OBJDIR)/Maus_MauTypeMonitor.o \
		###$(OBJDIR)/MausOI.o \
		###$(OBJDIR)/MausOM.o
				      
# Define own Include paths

### CINCLUDES += -I$(INCDIR) -I$(BLACK_INC) -I$(COMMON_INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC)
CINCLUDES += -I$(INCDIR) -I$(COMMON_INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) \
-I$(COREMW_SDK_INC) -I$(ACE_SDK_INC)

# Define own CFLAGS
#ifeq ($(COVERAGE),1)
#	CFLAGS += -O0
#else
#	CFLAGS += -Os
#endif

CFLAGS += -O -g -Wall -ansi

# Define TARGET
TARGET = maus.a

.PHONY: all
all: $(OBJDIR)/$(TARGET)
   
$(OBJDIR)/$(TARGET): $(OBJ)
	$(SILENT)$(ECHO) 'Creating static library: $(TARGET)'
	$(SILENT)$(AR) -rcs $(OBJDIR)/$(TARGET) $(OBJ)
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
	$(SILENT)$(RM) -r $(OBJDIR)/*

