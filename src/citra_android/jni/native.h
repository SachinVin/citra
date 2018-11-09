// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#define JNI_NATIVE_LIBRARY(FUNC_NAME) Java_org_citra_citra_1android_NativeLibrary_##FUNC_NAME

// Initialise and run the emulator
static int RunCitra(const std::string& path);

// Function calls from the Java side
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(UnPauseEmulation)(JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(PauseEmulation)(JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(StopEmulation)(JNIEnv* env, jobject obj);

JNIEXPORT jboolean JNICALL JNI_NATIVE_LIBRARY(IsRunning)(JNIEnv* env, jobject obj);

JNIEXPORT jboolean JNICALL JNI_NATIVE_LIBRARY(onGamePadEvent)(JNIEnv* env, jobject obj,
                                                              jstring jDevice, jint Button,
                                                              jint Action);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(onGamePadMoveEvent)(JNIEnv* env, jobject obj,
                                                              jstring jDevice, jint Axis, jfloat x,
                                                              jfloat y);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(onTouchEvent)(JNIEnv* env,jobject obj, jfloat x, jfloat y,
                                                        jboolean pressed);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(onTouchMoved)(JNIEnv* env, jobject obj, jfloat x,
                                                        jfloat y);

JNIEXPORT jintArray JNICALL JNI_NATIVE_LIBRARY(GetBanner)(JNIEnv* env,jobject obj, jstring jFile);

JNIEXPORT jstring JNICALL JNI_NATIVE_LIBRARY(GetTitle)(JNIEnv* env,jobject obj, jstring jFilename);

JNIEXPORT jstring JNICALL JNI_NATIVE_LIBRARY(GetDescription)(JNIEnv* env, jobject obj,
                                                             jstring jFilename);

JNIEXPORT jstring JNICALL JNI_NATIVE_LIBRARY(GetGameId)(JNIEnv* env, jobject obj,
                                                        jstring jFilename);

JNIEXPORT jint JNICALL JNI_NATIVE_LIBRARY(GetCountry)(JNIEnv* env, jobject obj, jstring jFilename);

JNIEXPORT jstring JNICALL JNI_NATIVE_LIBRARY(GetCompany)(JNIEnv* env, jobject obj,
                                                         jstring jFilename);

JNIEXPORT jlong JNICALL JNI_NATIVE_LIBRARY(GetFilesize)(JNIEnv* env, jobject obj,
                                                        jstring jFilename);

JNIEXPORT jstring JNICALL JNI_NATIVE_LIBRARY(GetVersionString)(JNIEnv* env, jobject obj);

JNIEXPORT jstring JNICALL JNI_NATIVE_LIBRARY(GetGitRevision)(JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(CreateConfigFile)(JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(SetProfiling)(JNIEnv* env, jobject obj, jboolean enable);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(WriteProfileResults)(JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(CacheClassesAndMethods)(JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(Run)(JNIEnv* env, jclass type, jstring path_);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(SurfaceChanged)(JNIEnv* env, jobject obj, jobject surf);

JNIEXPORT void JNICALL JNI_NATIVE_LIBRARY(SurfaceDestroyed)(JNIEnv* env, jobject obj);

JNIEXPORT jdoubleArray JNICALL JNI_NATIVE_LIBRARY(GetPerfStats)(JNIEnv* env, jclass type);

#ifdef __cplusplus
}
#endif
