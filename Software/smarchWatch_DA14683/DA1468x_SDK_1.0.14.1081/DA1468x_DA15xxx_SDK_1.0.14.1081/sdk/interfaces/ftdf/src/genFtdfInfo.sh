#!/bin/bash
# This script generates a file containing the actual FTDF release info
# consisting of a software version and a build time.
# The generated file is used for the FTDF_getReleaseInfo() API.
# This script is called in the Makefile
# usage: ./genFtdfInfo.sh ftdfInfo.h ftdfInfo.c
# NOTE: This file uses the header file ftdfInfo.h, containing the software version

HDR=$1
FILE=$2
DATE=$(date +'%d %b %y %H:%M')
ARCH=$(grep "FTDF_ARCHITECTURE_VERSION" $HDR | awk '{printf $3}')
BRANCH=$(grep "FTDF_BRANCH_VERSION" $HDR | awk '{printf $3}')
LOAD=$(grep "FTDF_LOAD_VERSION" $HDR | awk '{printf $3}')

echo "Generating FTDF info file in $FILE with build time $DATE"

echo "#include \"ftdfInfo.h\""                  >  $FILE
echo ""                                         >> $FILE
echo 'const char* FTDF_getUmacRelName( void )
{'                                              >> $FILE
echo "    return \"ftdf_$ARCH.$BRANCH.$LOAD\";" >> $FILE
echo '}

const char* FTDF_getUmacBuildTime( void )
{'                                              >> $FILE
echo "    return \"$DATE\";"                    >> $FILE
echo "}"                                        >> $FILE
