#!/bin/sh

cp $BASE_DIR/../custom-scripts/S41network-config $BASE_DIR/target/etc/init.d
chmod +x $BASE_DIR/target/etc/init.d/S41network-config

cp $BASE_DIR/../custom-scripts/S50hello $BASE_DIR/target/etc/init.d
chmod +x $BASE_DIR/target/etc/init.d/S50hello

cp $BASE_DIR/../programs/hello/helloi686 $BASE_DIR/target/usr/bin
cp $BASE_DIR/../programs/tp1/server.py $BASE_DIR/target/root