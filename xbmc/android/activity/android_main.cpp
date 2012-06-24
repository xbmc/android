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

#include "EventLoop.h"
#include "XBMCApp.h"
#include "unzip.h"

void setup_env(struct android_app* state)
{
  JavaVM* vm  = state->activity->vm;
  JNIEnv* env = state->activity->env;
  const char* temp;

  vm->AttachCurrentThread(&env, NULL);
  jobject activity = state->activity->clazz;
  jclass activityClass = env->GetObjectClass(activity);

  // get the path to the android system libraries
  jclass jSystem = env->FindClass("java/lang/System");
  jmethodID systemLib = env->GetStaticMethodID(jSystem, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
  jstring jSystemProperty = env->NewStringUTF("java.library.path");
  jstring jSystemLib = (jstring)env->CallStaticObjectMethod(jSystem, systemLib, jSystemProperty);
  temp = env->GetStringUTFChars(jSystemLib, NULL);
  setenv("XBMC_ANDROID_SYSTEM_LIBS", temp, 0);
  env->ReleaseStringUTFChars(jSystemLib, temp);
  env->DeleteLocalRef(jSystemLib);
  env->DeleteLocalRef(jSystem);
  env->DeleteLocalRef(jSystemProperty);

  // get the path to XBMC's data directory (usually /data/data/<app-name>)
  jmethodID getApplicationInfo = env->GetMethodID(activityClass, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
  jobject jApplicationInfo = env->CallObjectMethod(activity, getApplicationInfo);
  jclass applicationInfoClass = env->GetObjectClass(jApplicationInfo);
  jfieldID dataDir = env->GetFieldID(applicationInfoClass, "dataDir", "Ljava/lang/String;");
  jstring jDataDir = (jstring)env->GetObjectField(jApplicationInfo, dataDir);
  temp = env->GetStringUTFChars(jDataDir, NULL);
  setenv("XBMC_ANDROID_DATA", temp, 0);
  env->ReleaseStringUTFChars(jDataDir, temp);
  env->DeleteLocalRef(jDataDir);
  
  // get the path to where android extracts native libraries to
  jfieldID nativeLibraryDir = env->GetFieldID(applicationInfoClass, "nativeLibraryDir", "Ljava/lang/String;");
  jstring jNativeLibraryDir = (jstring)env->GetObjectField(jApplicationInfo, nativeLibraryDir);
  temp = env->GetStringUTFChars(jNativeLibraryDir, NULL);
  setenv("XBMC_ANDROID_LIBS", temp, 0);
  env->ReleaseStringUTFChars(jNativeLibraryDir, temp);
  env->DeleteLocalRef(jNativeLibraryDir);
  env->DeleteLocalRef(applicationInfoClass);
  env->DeleteLocalRef(jApplicationInfo);

  // get the path to the APK
  char apkPath[PATH_MAX];
  jmethodID getPackageResourcePath = env->GetMethodID(activityClass, "getPackageResourcePath", "()Ljava/lang/String;");
  jstring jApkPath = (jstring)env->CallObjectMethod(activity, getPackageResourcePath);
  temp = env->GetStringUTFChars(jApkPath, NULL);
  strcpy(apkPath, temp);
  setenv("XBMC_ANDROID_APK", apkPath, 0);
  env->ReleaseStringUTFChars(jApkPath, temp);
  env->DeleteLocalRef(jApkPath);
  
  // Get the path to the temp/cache directory
  char cacheDir[PATH_MAX];
  char tempDir[PATH_MAX];
  jmethodID getCacheDir = env->GetMethodID(activityClass, "getCacheDir", "()Ljava/io/File;");
  jobject jCacheDir = env->CallObjectMethod(activity, getCacheDir);

  jclass fileClass = env->GetObjectClass(jCacheDir);
  jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
  env->DeleteLocalRef(fileClass);

  jstring jCachePath = (jstring)env->CallObjectMethod(jCacheDir, getAbsolutePath);
  temp = env->GetStringUTFChars(jCachePath, NULL);
  strcpy(cacheDir, temp);
  strcpy(tempDir, temp);
  env->ReleaseStringUTFChars(jCachePath, temp);
  env->DeleteLocalRef(jCachePath);
  env->DeleteLocalRef(jCacheDir);

  strcat(tempDir, "/temp");
  setenv("XBMC_TEMP", tempDir, 0);

  strcat(cacheDir, "/apk");

  //cache assets from the apk
  extract_to_cache(apkPath, cacheDir);

  strcat(cacheDir, "/assets");
  setenv("XBMC_BIN_HOME", cacheDir, 0);
  setenv("XBMC_HOME"    , cacheDir, 0);

  // Get the path to the external storage by employing JNI
  // The path would actually be available from state->activity->externalDataPath (apart from a (known) bug in Android 2.3.x)
  // but calling getExternalFilesDir() will automatically create the necessary directories for us. We don't seem to have the
  // rights to create a directory in /mnt/sdcard/Android/data/ ourselfs.
  char storagePath[PATH_MAX];
  jmethodID getExternalFilesDir = env->GetMethodID(activityClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
  jobject jExternalDir = env->CallObjectMethod(activity, getExternalFilesDir, (jstring)NULL);

  jstring jExternalPath = (jstring)env->CallObjectMethod(jExternalDir, getAbsolutePath);
  temp = env->GetStringUTFChars(jExternalPath, NULL);
  strcpy(storagePath, temp);
  env->ReleaseStringUTFChars(jExternalPath, temp);
  env->DeleteLocalRef(jExternalPath);
  env->DeleteLocalRef(jExternalDir);

  // Check if we don't have a valid path yet
  if (strlen(storagePath) <= 0)
  {
    // Get the path to the internal storage by employing JNI
    // For more details see the comment on getting the path to the external storage
    jstring jstrName = env->NewStringUTF("org.xbmc");
    jmethodID getDir = env->GetMethodID(activityClass, "getDir", "(Ljava/lang/String;I)Ljava/io/File;");
    jobject jInternalDir = env->CallObjectMethod(activity, getDir, jstrName, 1 /* MODE_WORLD_READABLE */);
    env->DeleteLocalRef(jstrName);

    jstring jInternalPath = (jstring)env->CallObjectMethod(jInternalDir, getAbsolutePath);
    temp = env->GetStringUTFChars(jInternalPath, NULL);
    strcpy(storagePath, temp);
    env->ReleaseStringUTFChars(jInternalPath, temp);
    env->DeleteLocalRef(jInternalPath);
    env->DeleteLocalRef(jInternalDir);
  }
  
  env->DeleteLocalRef(activityClass);

  // Check if we have a valid home path
  if (strlen(storagePath) > 0)
    setenv("HOME", storagePath, 0);
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
    CXBMCApp::android_printf("android_main: setup failed");

  CXBMCApp::android_printf("android_main: Exiting");
  state->activity->vm->DetachCurrentThread();
  // We need to call exit() so that all loaded libraries are properly unloaded
  // otherwise on the next start of the Activity android will simple re-use
  // those loaded libs in the state they were in when we quit XBMC last time
  // which will lead to crashes because of global/static classes that haven't
  // been properly uninitialized
  exit(0);
}
