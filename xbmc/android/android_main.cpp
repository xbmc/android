/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include <android_native_app_glue.h>
#include <jni.h>

#include "android_utils.h"
#include "xbmc_log.h"
#include "EventLoop.h"
#include "XBMCApp.h"

/*static int android_printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int result = __android_log_vprint(ANDROID_LOG_VERBOSE, "XBMC", format, args);
  va_end(args);
  return result;
}*/

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

  // Get the path to the external storage by employing JNI
  // The path would actually be available from state->activity->externalDataPath (apart from a (known) bug in Android 2.3.x)
  // but calling getExternalFilesDir() will automatically create the necessary directories for us. We don't seem to have the
  // rights to create a directory in /mnt/sdcard/Android/data/ ourselfs.
  jmethodID getExternalFilesDir = env->GetMethodID(activityClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
  jobject jExternalDir = env->CallObjectMethod(activity, getExternalFilesDir, (jstring)NULL);

  jstring jExternalPath = (jstring)env->CallObjectMethod(jExternalDir, getAbsolutePath);
  const char *externalPath = env->GetStringUTFChars(jExternalPath, NULL);
  strcpy(cstr, externalPath);
  env->ReleaseStringUTFChars(jExternalPath, externalPath);

  // Check if we don't have a valid path yet
  if (strlen(cstr) <= 0)
  {
    // Get the path to the internal storage by employing JNI
    // For more details see the comment on getting the path to the external storage
    jstring jstrName = env->NewStringUTF("org.xbmc");
    jmethodID getDir = env->GetMethodID(activityClass, "getDir", "(Ljava/lang/String;I)Ljava/io/File;");
    jobject jInternalDir = env->CallObjectMethod(activity, getDir, jstrName, 1 /* MODE_WORLD_READABLE */);

    jstring jInternalPath = (jstring)env->CallObjectMethod(jInternalDir, getAbsolutePath);
    const char *internalPath = env->GetStringUTFChars(jInternalPath, NULL);
    strcpy(cstr, internalPath);
    env->ReleaseStringUTFChars(jInternalPath, internalPath);
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

  setup_env(state);

  CEventLoop eventLoop(state);
  CXBMCApp xbmcApp(state->activity);
  if (xbmcApp.isValid())
    eventLoop.run(xbmcApp, xbmcApp);
  else
    android_printf("android_main: setup failed");

  android_printf("android_main: Exiting");
  state->activity->vm->DetachCurrentThread();
  // We need to call exit() so that all loaded libraries are properly unloaded
  // otherwise on the next start of the Activity android will simple re-use
  // those loaded libs in the state they were in when we quit XBMC last time
  // which will lead to crashes because of global/static classes that haven't
  // been properly uninitialized
  exit(0);
}
