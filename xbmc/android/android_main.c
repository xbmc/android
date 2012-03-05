#include <android_native_app_glue.h>
#include <android/log.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>

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

void preload()
{
  tryopen("/data/data/org.xbmc/lib/libsqlite.so");
  tryopen("/data/data/org.xbmc/lib/libyajl.so");
  tryopen("/data/data/org.xbmc/lib/libtiff.so");
  tryopen("/data/data/org.xbmc/lib/libjpeg.so");
  tryopen("/data/data/org.xbmc/lib/libgpg-error.so");
  tryopen("/data/data/org.xbmc/lib/libgcrypt.so");
  tryopen("/data/data/org.xbmc/lib/libiconv.so");
  tryopen("/data/data/org.xbmc/lib/libfreetype.so");
  tryopen("/data/data/org.xbmc/lib/libfontconfig.so");
  tryopen("/data/data/org.xbmc/lib/libfribidi.so");
  tryopen("/data/data/org.xbmc/lib/libsqlite3.so");
  tryopen("/data/data/org.xbmc/lib/libpng12.so");
  tryopen("/data/data/org.xbmc/lib/libpcre.so");
  tryopen("/data/data/org.xbmc/lib/libsamplerate.so");
  tryopen("/data/data/org.xbmc/lib/libpython2.6.so");
  tryopen("/data/data/org.xbmc/lib/libpcrecpp.so");
  tryopen("/data/data/org.xbmc/lib/libzip.so");

  // gdb sleeps before attaching for some reason. let it attach before we try
  // to load libxbmc.
  sleep(1);
}

extern void android_main(struct android_app* state)
{
  preload();

  // make sure that the linker doesn't strip out our glue
  app_dummy();

  void* soHandle;
  soHandle = tryopen("/data/data/org.xbmc/lib/libxbmc.so");

  typedef void (*xbmc_t)(struct android_app*);
  xbmc_t xbmcinit = (xbmc_t)dlsym (soHandle, "android_init");
  if (xbmcinit == NULL)
  {
    __android_log_print(ANDROID_LOG_VERBOSE, "XBMC", "could not find function. Error: %s",dlerror());
   exit(1);
  }
  xbmcinit(state);

  //Now to relax.
  while(1)
  {
    usleep(1);
  }
}

