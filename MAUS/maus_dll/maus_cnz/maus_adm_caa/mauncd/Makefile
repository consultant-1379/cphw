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

CXCDIR = ../../maus1_cxc

APNAME = cphw_maus1d
APEXE  = $(OUTDIR)/$(APNAME)

# Define COMMON  paths
COMMON_DIR     = ../../maus_adm_caa/common
COMMON_INCDIR  = $(COMMON_DIR)/inc
COMMON_OBJDIR  = $(COMMON_DIR)/obj
COMMON_LIB     = $(COMMON_OBJDIR)/libcommon.a

# Define MAUS  paths
MAUS_DIR     = ../../maus_adm_caa/maus
MAUS_INCDIR  = $(MAUS_DIR)/inc
MAUS_OBJDIR  = $(MAUS_DIR)/obj
MAUS_LIB     = $(MAUS_OBJDIR)/maus.a

# Define objs want to build
OBJ = $(OBJDIR)/main_ncd.o
				      

CINCLUDES += -I$(INCDIR) -I$(COMMON_INCDIR) -I$(MAUS_INCDIR) -I$(BOOST_SDK_INC) 
CINCLUDES += -I$(AP_SDK_INC) -I$(COREMW_SDK_INC) -I$(ACE_SDK_INC)

LIBSDIR += -L$(BOOST_SDK_LIB) -L$(AP_SDK_LIB) -L$(LIB_LOGCPLUS_SDK_LIB) -L$(COREMW_SDK_LIB)

LIBS += -lboost_regex -lboost_system -lboost_thread -lacs_apgcc -lacs_dsd -lacs_prc -lcrypto
LIBS += -lacs_tra -lacs_csapi -lacs_aeh -lcap -ldl -lpthread -lrt -lSaImmOm

LDFLAGS += -rdynamic -export-dynamic
LDFLAGS += -Wl,-rpath-link,$(AP_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(COREMW_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(SEC_CREDU_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_LOGCPLUS_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(LIB_XERCES_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(BOOST_SDK_LIB)
LDFLAGS += -Wl,-rpath-link,$(ACE_SDK_LIB)

# Define own CFLAGS
#ifeq ($(COVERAGE),1)
#	CFLAGS += -O0
	CFLAGS += -O
#else
#	CFLAGS += -Os
#endif

EXTRA_LINKER_FLAGS = -Wl,--start-group

#CFLAGS += -g -Wall -ansi -DNDEBUG -DLOCAL_BUILD
CFLAGS += -fPIC -g -Wall -ansi  

.PHONY: all
all: $(OUTDIR)/$(APNAME)

.PHONY: common
common:
# These command will be executed in another shell (not the current shell)
	cd $(COMMON_DIR); $(MAKE) $(MFLAGS) all

.PHONY: maus
maus:
# These command will be executed in another shell (not the current shell)
	cd $(MAUS_DIR); $(MAKE) $(MFLAGS) all
   
$(OUTDIR)/$(APNAME): $(OBJ) common maus
	$(SILENT)$(ECHO) 'Linking $@... '
	$(SILENT)$(CC) -o $(APEXE) $(OBJ) $(LDFLAGS) $(LIBSDIR) $(LIBS) $(MAUS_LIB) $(COMMON_LIB)
	$(SILENT)$(ECHO) '$(APEXE) is created'
	$(call stripp,$(APNAME))
	$(SEPARATOR_STR)
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
	cd $(MAUS_DIR); $(MAKE) $(MFLAGS) clean;
	$(SILENT)$(RM) -r $(OBJDIR)/*

.PHONY: distclean
distclean: clean
	$(SILENT)$(RM) -r $(OUTDIR)/$(APNAME)
	$(SILENT)$(RM) -r $(DEBUGDIR)/$(APNAME)_dbg

