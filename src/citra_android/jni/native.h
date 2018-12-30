// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

// Initialise and run the emulator
static int RunCitra(const std::string& path);

// Function calls from the Java side
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_UnPauseEmulation(JNIEnv* env,
                                                                                    jobject obj);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_PauseEmulation(JNIEnv* env,
                                                                                  jobject obj);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_StopEmulation(JNIEnv* env,
                                                                                 jobject obj);

JNIEXPORT jboolean JNICALL Java_org_citra_citra_1android_NativeLibrary_IsRunning(JNIEnv* env,
                                                                                 jobject obj);

JNIEXPORT jboolean JNICALL Java_org_citra_citra_1android_NativeLibrary_onGamePadEvent(
    JNIEnv* env, jobject obj, jstring jDevice, jint Button, jint Action);

JNIEXPORT jboolean JNICALL Java_org_citra_citra_1android_NativeLibrary_onGamePadMoveEvent(
    JNIEnv* env, jobject obj, jstring jDevice, jint Axis, jfloat x, jfloat y);

JNIEXPORT jboolean JNICALL Java_org_citra_citra_1android_NativeLibrary_onGamePadAxisEvent(
    JNIEnv* env, jobject obj, jstring jDevice, jint axis_id, jfloat axis_val);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_onTouchEvent(JNIEnv* env,
                                                                                jobject obj,
                                                                                jfloat x, jfloat y,
                                                                                jboolean pressed);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_onTouchMoved(JNIEnv* env,
                                                                                jobject obj,
                                                                                jfloat x, jfloat y);

JNIEXPORT jintArray JNICALL Java_org_citra_citra_1android_NativeLibrary_GetBanner(JNIEnv* env,
                                                                                  jobject obj,
                                                                                  jstring jFile);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetTitle(JNIEnv* env,
                                                                               jobject obj,
                                                                               jstring jFilename);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetDescription(
    JNIEnv* env, jobject obj, jstring jFilename);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetGameId(JNIEnv* env,
                                                                                jobject obj,
                                                                                jstring jFilename);

JNIEXPORT jint JNICALL Java_org_citra_citra_1android_NativeLibrary_GetCountry(JNIEnv* env,
                                                                              jobject obj,
                                                                              jstring jFilename);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetCompany(JNIEnv* env,
                                                                                 jobject obj,
                                                                                 jstring jFilename);

JNIEXPORT jlong JNICALL Java_org_citra_citra_1android_NativeLibrary_GetFilesize(JNIEnv* env,
                                                                                jobject obj,
                                                                                jstring jFilename);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetVersionString(JNIEnv* env,
                                                                                       jobject obj);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetGitRevision(JNIEnv* env,
                                                                                     jobject obj);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SaveScreenShot(JNIEnv* env,
                                                                                  jobject obj);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_eglBindAPI(JNIEnv* env,
                                                                              jobject obj,
                                                                              jint api);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetConfig(
    JNIEnv* env, jobject obj, jstring jFile, jstring jSection, jstring jKey, jstring jDefault);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SetConfig(
    JNIEnv* env, jobject obj, jstring jFile, jstring jSection, jstring jKey, jstring jValue);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SetFilename(JNIEnv* env,
                                                                               jobject obj,
                                                                               jstring jFile);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SaveState(JNIEnv* env,
                                                                             jobject obj, jint slot,
                                                                             jboolean wait);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SaveStateAs(JNIEnv* env,
                                                                               jobject obj,
                                                                               jstring path,
                                                                               jboolean wait);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_LoadState(JNIEnv* env,
                                                                             jobject obj,
                                                                             jint slot);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_LoadStateAs(JNIEnv* env,
                                                                               jobject obj,
                                                                               jstring path);

JNIEXPORT void JNICALL
Java_org_citra_citra_1android_services_DirectoryInitializationService_CreateUserDirectories(
    JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SetUserDirectory(
    JNIEnv* env, jobject obj, jstring jDirectory);

JNIEXPORT void JNICALL
Java_org_citra_citra_1android_services_DirectoryInitializationService_SetSysDirectory(
    JNIEnv* env, jclass type, jstring path_);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetUserDirectory(JNIEnv* env,
                                                                                       jobject obj);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SetSysDirectory(JNIEnv* env,
                                                                                   jobject obj,
                                                                                   jstring path);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_CreateConfigFile();

JNIEXPORT jint JNICALL Java_org_citra_citra_1android_NativeLibrary_DefaultCPUCore(JNIEnv* env,
                                                                                  jobject obj);
JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SetProfiling(JNIEnv* env,
                                                                                jobject obj,
                                                                                jboolean enable);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_WriteProfileResults(JNIEnv* env,
                                                                                       jobject obj);

JNIEXPORT void JNICALL
Java_org_citra_citra_1android_NativeLibrary_CacheClassesAndMethods(JNIEnv* env, jobject obj);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_Run__Ljava_lang_String_2(
    JNIEnv* env, jclass type, jstring path_);

JNIEXPORT void JNICALL
Java_org_citra_citra_1android_NativeLibrary_Run__Ljava_lang_String_2Ljava_lang_String_2Z(
    JNIEnv* env, jobject obj, jstring jFile, jstring jSavestate, jboolean jDeleteSavestate);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SurfaceChanged(JNIEnv* env,
                                                                                  jobject obj,
                                                                                  jobject surf);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SurfaceDestroyed(JNIEnv* env,
                                                                                    jobject obj);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_InitGameIni(JNIEnv* env,
                                                                               jclass type,
                                                                               jstring gameID_);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_SetUserSetting(
    JNIEnv* env, jclass type, jstring gameID_, jstring Section_, jstring Key_, jstring Value_);

JNIEXPORT jstring JNICALL Java_org_citra_citra_1android_NativeLibrary_GetUserSetting(
    JNIEnv* env, jclass type, jstring gameID_, jstring Section_, jstring Key_);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_ChangeDisc(JNIEnv* env,
                                                                              jclass type,
                                                                              jstring path_);

JNIEXPORT void JNICALL Java_org_citra_citra_1android_NativeLibrary_RefreshWiimotes(JNIEnv* env,
                                                                                   jclass type);

JNIEXPORT jint JNICALL Java_org_citra_citra_1android_NativeLibrary_GetPlatform(JNIEnv* env,
                                                                               jclass type,
                                                                               jstring filename_);

JNIEXPORT jint JNICALL Java_org_citra_citra_1android_NativeLibrary_GetPlatform(JNIEnv* env,
                                                                               jclass type,
                                                                               jstring filename_);

JNIEXPORT jdoubleArray JNICALL
Java_org_citra_citra_1android_NativeLibrary_GetPerfStats(JNIEnv* env, jclass type);

#ifdef __cplusplus
}
#endif