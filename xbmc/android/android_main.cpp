#include <android_native_app_glue.h>
#include <android/log.h>
#include <jni.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "xbmc.h"

static int android_printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int result = __android_log_vprint(ANDROID_LOG_VERBOSE, "XBMC", format, args);
  va_end(args);
  return result;
}

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

void setup_env(struct android_app* state)
{
  JavaVM* vm  = state->activity->vm;
  JNIEnv* env = state->activity->env;

  vm->AttachCurrentThread(&env, NULL);
  jobject activity = state->activity->clazz;
  jclass activityClass = env->GetObjectClass(activity);

  char cstr[1024];
  jmethodID getPackageResourcePath = env->GetMethodID(activityClass, "getPackageResourcePath", "()Ljava/lang/String;");
  jstring jApkPath = (jstring)env->CallObjectMethod(activity, getPackageResourcePath);
  const char* apkPath = env->GetStringUTFChars(jApkPath, NULL);
  strcpy(cstr, apkPath);
  env->ReleaseStringUTFChars(jApkPath, apkPath);
  strcat(cstr, "/assets");
  setenv("XBMC_BIN_HOME", cstr, 0);
  setenv("XBMC_HOME"    , cstr, 0);
  
  memset(cstr, 0, 1024);

  // Get the path to the temp/cache directory
  jmethodID getCacheDir = env->GetMethodID(activityClass, "getCacheDir", "()Ljava/io/File;");
  jobject jCacheDir = env->CallObjectMethod(activity, getCacheDir);
  
  jclass fileClass = env->GetObjectClass(jCacheDir);
  jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
  
  jstring jCachePath = (jstring)env->CallObjectMethod(jCacheDir, getAbsolutePath);
  const char* cachePath = env->GetStringUTFChars(jCachePath, NULL);
  setenv("XBMC_TEMP", cachePath, 0);
  env->ReleaseStringUTFChars(jCachePath, cachePath);
  
  // Get the path to the external storage
  if (state->activity->externalDataPath != NULL)
    strcpy(cstr, state->activity->externalDataPath);
  else
  {
    // We need to employ JNI to get the path because of a (known) bug in Android 2.3.x
    jmethodID getExternalFilesDir = env->GetMethodID(activityClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
    jobject jExternalDir = env->CallObjectMethod(activity, getExternalFilesDir, (jstring)NULL);
    
    jstring jExternalPath = (jstring)env->CallObjectMethod(jExternalDir, getAbsolutePath);
    const char *externalPath = env->GetStringUTFChars(jExternalPath, NULL);
    strcpy(cstr, externalPath);
    env->ReleaseStringUTFChars(jExternalPath, externalPath);
  }

  // Check if we don't have a valid path yet
  if (strlen(cstr) <= 0)
  {
    // Get the path to the internal storage
    if (state->activity->internalDataPath != NULL)
      strcpy(cstr, state->activity->internalDataPath);
    else
    {
      // We need to employ JNI to get the path because of a (known) bug in Android 2.3.x
      jstring jstrName = env->NewStringUTF("org.xbmc");
      jmethodID getDir = env->GetMethodID(activityClass, "getDir", "(Ljava/lang/String;I)Ljava/io/File;");
      jobject jInternalDir = env->CallObjectMethod(activity, getDir, jstrName, 1 /* MODE_WORLD_READABLE */);
    
      jstring jInternalPath = (jstring)env->CallObjectMethod(jInternalDir, getAbsolutePath);
      const char *internalPath = env->GetStringUTFChars(jInternalPath, NULL);
      strcpy(cstr, internalPath);
      env->ReleaseStringUTFChars(jInternalPath, internalPath);
    }
  }
  
  // Check if we have a valid home path
  if (strlen(cstr) > 0)
    setenv("HOME", cstr, 0);
  else
    setenv("HOME", getenv("XBMC_TEMP"), 0);
}

extern void android_main(struct android_app* state)
{
  // make sure that the linker doesn't strip out our glue
  app_dummy();

  preload();
  setup_env(state);

  void* soHandle;
  soHandle = tryopen("/data/data/org.xbmc/lib/libxbmc.so");

  // fetch xbmc entry points
  XBMC_Run_t XBMC_Run;
  XBMC_Initialize_t XBMC_Initialize;

  XBMC_Run = (XBMC_Run_t)dlsym(soHandle, "XBMC_Run");
  XBMC_Initialize = (XBMC_Initialize_t)dlsym(soHandle, "XBMC_Initialize");
  if (XBMC_Run == NULL || XBMC_Initialize == NULL)
  {
    android_printf("could not find functions. Error: %s\n", dlerror());
    exit(1);
  }

  // XBMC_PLATFORM MUST exist for app lifetime.
  static XBMC_PLATFORM platform = {XBMCRunNull, 0};
  // stardard platform stuff
  platform.flags  = XBMCRunAsApp;
  platform.log_name = "XBMC";
  // android specific
  if(state && state->window)
  {
    platform.width  = ANativeWindow_getWidth(state->window);
    platform.height = ANativeWindow_getHeight(state->window);
    platform.format = ANativeWindow_getFormat(state->window);
  }
  platform.window_type = state->window;
  // callbacks into android
  platform.android_printf = &android_printf;
  platform.android_setBuffersGeometry = &ANativeWindow_setBuffersGeometry;

  int status = 0;
  status = XBMC_Initialize(&platform, NULL, NULL);
  if (!status)
  {
    try
    {
      status = XBMC_Run();
    }
    catch(...)
    {
      android_printf("ERROR: Exception caught on main loop. Exiting\n");
    }
  }
  else
    android_printf("ERROR: XBMC_Initialize failed. Exiting\n");

  if (status == 0)
    android_printf("DEBUG: Exiting gracefully.\n");

  state->activity->vm->DetachCurrentThread();
  return;
}
