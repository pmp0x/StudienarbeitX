#!/bin/bash

# SCRIPT=`readlink -f $0`
# LIB_MAPLE_HOME = '/Users/marcus/Dropbox/Studienarbeit/SourcecodeX/Library/libmaple'
SCRIPTDIR=`pwd`
USER_MODULES="$SCRIPTDIR/Library/TinyWebServer $SCRIPTDIR/Library/WiFlySerial $SCRIPTDIR" make -j4 -f "$LIB_MAPLE_HOME/Makefile" $@