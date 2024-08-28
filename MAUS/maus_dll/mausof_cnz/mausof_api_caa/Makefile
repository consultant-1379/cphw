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
# There is no need to change
#SRCDIR = src
#OBJDIR = obj
#OUTDIR = $(CXCDIR)/bin
#INCDIR = inc

include /vobs/IO_Developments/Tools/linux_conf_file/common.mk

CXCDIR ?= ../mausof_cxc
OUTDIR   = $(CXCDIR)/bin/lib_ext

LIB_NAME = mauso
LIB_REL_MAJOR ?= 1
LIB_REL_MINOR ?= 0
LIB_REL_BUILD ?= 0
LIB_VER ?= $(LIB_REL_MAJOR).$(LIB_REL_MINOR).$(LIB_REL_BUILD)
LIB_FILENAME = lib$(LIB_NAME).so.$(LIB_VER)
LIB_SONAME = lib$(LIB_NAME).so.$(LIB_REL_MAJOR)
LIB_LINKER_NAME = lib$(LIB_NAME).so

CPHW_MAUSO_APLIB = $(OUTDIR)/$(LIB_FILENAME)
CPHW_MAUSO_APLIB_SO = $(OUTDIR)/$(LIB_LINKER_NAME)


# Define objs want to build
MAUSO_OBJ = $(OBJDIR)/MausAPI.o 
      
# Define own Include paths
CINCLUDES += -I$(INCDIR) -I$(BOOST_SDK_INC) -I$(AP_SDK_INC) -I$(3PP_SDK_PATH)

# Define own lib paths
LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB)

# Define own Libs
#LIBS += -lboost_regex -lboost_system -lboost_thread -lacs_apgcc -llog4cplus -lacs_tra -lacs_csapi -lacs_aeh -lcap

LIBS += -ldl

# Define own CFLAGS
ifeq ($(COVERAGE),1)
	CFLAGS += -O0
else
	CFLAGS += -Os
endif

CFLAGS += -fPIC -g -Wall -ansi
LDFLAGS += -shared


.PHONY: all
all: $(CPHW_MAUSO_APLIB)
		@echo "Hello all is called in MAUSO"
   
$(CPHW_MAUSO_APLIB): $(MAUSO_OBJ) 
	@echo "Hello MAUSO"
	$(NEW_LINE)
	$(SEPARATOR_STR)
	$(SILENT)$(ECHO) 'Creating library: $(LIB_FILENAME)'
	$(CC) -o $(CPHW_MAUSO_APLIB) $(MAUSO_OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS)
	$(CC) --version
	$(call stripp,$(LIB_FILENAME))	
	$(SILENT)$(ECHO) 'Finished building target: $@'
	$(SEPARATOR_STR)
	$(NEW_LINE)


# Include dependecies in order to build when header file changes
DEPS = $(MAUSO_OBJ:.o=.d)
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


