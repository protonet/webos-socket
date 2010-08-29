#!/bin/bash

if [ ! -e plugin ];then
cd ..
fi

STAGING_DIR=STAGING/com.protonet.sockettest

rm -rf $STAGING_DIR
rm *.ipk
mkdir -p $STAGING_DIR
rsync -r --exclude=.DS_Store --exclude=.svn mojo/ $STAGING_DIR/
./mac/buildit_for_device.sh
cp socket_plugin $STAGING_DIR
echo "filemode.755=socket_plugin" > $STAGING_DIR/package.properties
palm-package $STAGING_DIR
