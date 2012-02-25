#!/bin/sh

if [ "$1" = "i" ]; then
  adb install bin/NativeActivity-debug.apk
fi

if [ "$1" = "u" ]; then
  adb uninstall com.example.native_activity
fi

if [ "$1" = "b" ]; then
  android update project -p .
  /mnt/bighdd/Documents/xbmc/android-ndk-r6b/ndk-build
  ant debug
fi

if [ "$1" = "n" ]; then
  /mnt/bighdd/Documents/xbmc/android-ndk-r6b/ndk-build
fi
  
if [ "$1" = "d" ]; then
  /mnt/bighdd/Documents/xbmc/android-ndk-r6b/ndk-gdb --start
fi

if [ "$1" = "a" ]; then
  ./doit.sh u
  ./doit.sh b
  ./doit.sh i
  ./doit.sh d
fi



  





