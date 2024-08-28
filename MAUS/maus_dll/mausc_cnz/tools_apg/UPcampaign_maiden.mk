##
##

## This only generates MAUSC UP packages maiden installtion
##
##

MAUSC_TOOL ?= /vobs/mas/mas_anz/maus_dll/mausc_cnz
IODEV_VOB ?= /vobs/IO_Developments

### ///////////////////////////////////////////////////////////////////////

define up_creation_maiden
$(SILENT)$(ECHO) 'UP Package Creating for maiden MAUSC ...'
pushd $(IODEV_VOB)/Tools/CI/UP;\
$(MAUSC_TOOL)/tools_apg/createUPcampaign_maiden_apg.sh -v -t "$1:$2:$3:$4:$5:$6";\
if [ $$? != 0 ]; \
	then exit 1;\
fi; \
popd;
endef
