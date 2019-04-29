#! /bin/bash

if [ `echo $PWD | sed 's%/.*/%%'` != "drv" ]
then
    echo "genDoc.sh must be called from the \"drv\" directory"
    exit 1
fi

AV=`grep FTDF_ARCHITECTURE_VERSION src/ftdfInfo.h | cut -c 35-`
BV=`grep FTDF_BRANCH_VERSION src/ftdfInfo.h | cut -c 35-`
LV=`grep FTDF_LOAD_VERSION src/ftdfInfo.h | cut -c 35-`

sed "s/FTDF_VERSION/${AV}.${BV}.${LV}/" doc/Doxyfile.tmpl > doc/Doxyfile
doxygen doc/Doxyfile > doc/dgenoutput 2>&1
