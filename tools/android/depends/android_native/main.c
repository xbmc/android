#include <android_native_app_glue.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>

#include <jni.h>

#include <android/log.h>
#include <errno.h>
#include <stdbool.h>
//#include "xbmc.h"

void* tryopen(const char *path)
{
  void *handle;
  handle = dlopen(path, RTLD_NOW);
  if (!handle)
  {
      __android_log_print(ANDROID_LOG_VERBOSE, "XBMC", "could not load %s. Error: %s",path, dlerror());
   exit(1);
  }
  __android_log_print(ANDROID_LOG_VERBOSE, "XBMC", "loaded lib: %s",path);
  return handle;
}

extern void android_main(struct android_app* state)
{
  // make sure that the linker doesn't strip out our glue
  app_dummy();

  tryopen("/data/data/com.nativetest/lib/libsqlite.so");
  tryopen("/data/data/com.nativetest/lib/libyajl.so");
  tryopen("/data/data/com.nativetest/lib/libtiff.so");
  tryopen("/data/data/com.nativetest/lib/libjpeg.so");
  tryopen("/data/data/com.nativetest/lib/libgpg-error.so");
  tryopen("/data/data/com.nativetest/lib/libgcrypt.so");
  tryopen("/data/data/com.nativetest/lib/libiconv.so");
  tryopen("/data/data/com.nativetest/lib/libfreetype.so");
  tryopen("/data/data/com.nativetest/lib/libfontconfig.so");
  tryopen("/data/data/com.nativetest/lib/libfribidi.so");
  tryopen("/data/data/com.nativetest/lib/libsqlite3.so");
  tryopen("/data/data/com.nativetest/lib/libpng12.so");
  tryopen("/data/data/com.nativetest/lib/libpcre.so");
  tryopen("/data/data/com.nativetest/lib/libsamplerate.so");
  tryopen("/data/data/com.nativetest/lib/libpython2.6.so");
  tryopen("/data/data/com.nativetest/lib/libpcre.so");

  // gdb sleeps before attaching for some reason. let it attach before we try
  // to load libxbmc.
  sleep(1);

  void* soHandle;
  soHandle = tryopen("/data/data/com.nativetest/lib/libxbmc.so");

  typedef void (*xbmc_t)(unsigned int, const char*, int, const char**);
  xbmc_t xbmcinit = (xbmc_t)dlsym (soHandle, "XBMC_Init");
  if (xbmcinit == NULL)
  {
    __android_log_print(ANDROID_LOG_VERBOSE, "XBMC", "could not find function. Error: %s",dlerror());
   exit(1);
  }
  xbmcinit(7,"xbmc",0,NULL);

  //Now to relax.
  while(1)
  {
    usleep(1);
  }
}

