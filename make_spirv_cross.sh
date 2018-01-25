#!/bin/bash
TOP=$(pwd)
#clone
git clone https://github.com/KhronosGroup/SPIRV-Cross.git spriv_cross_tmp
#source
SOURCE_DIR=$TOP/spriv_cross_tmp
BUILD_DIR=$SOURCE_DIR/buind
export INSTALL_DIR=$SOURCE_DIR/install
# #build
mkdir -p $BUILD_DIR
mkdir -p $INSTALL_DIR
cd $BUILD_DIR
#init
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" $SOURCE_DIR
#make
make -j4 install
#back
cd $TOP
#copy all to diplib
if [[ "$OSTYPE" == "linux-gnu" ]] ; then
OS=linux
elif [[ "$OSTYPE" == "darwin"* ]] ; then
OS=macOS
fi
#copy
cp $INSTALL_DIR/lib/*.a $TOP/dependencies/$OS/lib/ 
cp -r $INSTALL_DIR/include/ $TOP/dependencies/$OS/include/
#delete all
yes | rm -R $SOURCE_DIR
