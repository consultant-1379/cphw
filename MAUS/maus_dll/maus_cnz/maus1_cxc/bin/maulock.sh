#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       maulock.sh
# Description:
#       A script to wrap the invocation of cfeted from the COM CLI.
# Note:
#	None.
##
# Usage:
#	None.
##
# Output:
#       None.
##
# Changelog:
# - Wed Oct 3 2012 - uabmagn
#	First version.
##

/usr/bin/sudo /opt/ap/cphw/bin/maulock "$@"

exit $?
