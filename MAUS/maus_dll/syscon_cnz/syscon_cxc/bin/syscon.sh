#!/bin/bash
##
# ------------------------------------------------------------------------
#     Copyright (C) 2012 Ericsson AB. All rights reserved.
# ------------------------------------------------------------------------
##
# Name:
#       syscon.sh
# Description:
#       A script to wrap the invocation of syscon from the COM CL or bash shell
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

/usr/bin/sudo /opt/ap/cphw/bin/syscon "$@"
retval=$?

if [ "$retval" -eq 0 ]
then
exit 0
fi

/usr/bin/sudo /opt/ap/cphw/bin/syscon 9999

exit $retval


