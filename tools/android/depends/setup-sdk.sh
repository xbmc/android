#!/bin/bash

SCRIPT_PATH=$(cd `dirname $0` && pwd)

#Edit these
NDKROOT=/opt/android/aml/android-ndk-r7-crystax-5.beta2
SDKROOT=/opt/android/aml/android-sdk-linux
TARBALLS_LOCATION=/opt/xbmc-tarballs
TOOLCHAIN=/opt/android/aml/android-ndk-r7-crystax-5.beta2-toolchain
XBMCPREFIX=/opt/xbmc-deps
HOST=arm-linux-androideabi
PLATFORM=armeabi-v7a
PLATFORM_FLAGS="-march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__ -DANDROID -Os"

#
#
mkdir -p $XBMCPREFIX/$PLATFORM $TARBALLS_LOCATION
sudo chown -R $USER:$USER $XBMCPREFIX/$PLATFORM $TARBALLS_LOCATION
mkdir -p $XBMCPREFIX/$PLATFORM/lib $XBMCPREFIX/$PLATFORM/include

#
#
echo "NDKROOT=$NDKROOT"                                                >  $SCRIPT_PATH/Makefile.include
echo "SDKROOT=$SDKROOT"                                                >> $SCRIPT_PATH/Makefile.include
echo "XBMCPREFIX=$XBMCPREFIX"                                          >> $SCRIPT_PATH/Makefile.include
echo "HOST=$HOST"                                                      >> $SCRIPT_PATH/Makefile.include
echo "PLATFORM=$PLATFORM"                                              >> $SCRIPT_PATH/Makefile.include
echo "PREFIX=$XBMCPREFIX/\$(PLATFORM)"                                 >> $SCRIPT_PATH/Makefile.include
echo "PLATFORM_FLAGS=$PLATFORM_FLAGS"                                  >> $SCRIPT_PATH/Makefile.include
echo "TOOLCHAIN=$TOOLCHAIN"                                            >> $SCRIPT_PATH/Makefile.include
echo "BASE_URL=http://mirrors.xbmc.org/build-deps/darwin-libs"         >> $SCRIPT_PATH/Makefile.include
echo "TARBALLS_LOCATION=$TARBALLS_LOCATION"                            >> $SCRIPT_PATH/Makefile.include
echo "RETRIEVE_TOOL=/usr/bin/curl"                                     >> $SCRIPT_PATH/Makefile.include
echo "RETRIEVE_TOOL_FLAGS=-Ls --create-dirs -f --output \$(TARBALLS_LOCATION)/\$(ARCHIVE)" >> $SCRIPT_PATH/Makefile.include
echo "ARCHIVE_TOOL=/bin/tar"                                           >> $SCRIPT_PATH/Makefile.include
echo "ARCHIVE_TOOL_FLAGS=-C \$(PLATFORM) --strip-components=1 -xf"     >> $SCRIPT_PATH/Makefile.include
echo "JOBS=$((`grep -c processor /proc/cpuinfo` -1))"                  >> $SCRIPT_PATH/Makefile.include
