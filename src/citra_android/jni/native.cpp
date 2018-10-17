#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>

// This needs to be included before getopt.h because the latter #defines symbols used by it
#include "common/microprofile.h"

#include <getopt.h>

#include <android/native_window_jni.h>
#include <jni.h>

#include "citra_android/jni/button_manager.h"
#include "citra_android/jni/config.h"
#include "citra_android/jni/emu_window/emu_window.h"
#include "citra_android/jni/game_info.h"
#include "citra_android/jni/native.h"
#include "common/common_paths.h"
#include "common/file_util.h"
#include "common/logging/backend.h"
#include "common/logging/filter.h"
#include "common/logging/log.h"
#include "common/scm_rev.h"
#include "common/scope_exit.h"
#include "common/string_util.h"
#include "core/core.h"
#include "core/file_sys/cia_container.h"
#include "core/frontend/applets/default_applets.h"
#include "core/gdbstub/gdbstub.h"
#include "core/hle/service/am/am.h"
#include "core/loader/loader.h"
#include "core/movie.h"
#include "core/settings.h"
#include "network/network.h"

JavaVM* g_java_vm;

namespace {
ANativeWindow* s_surf;

jclass s_jni_class;
jmethodID s_jni_method_alert;

EmuWindow_Android* emu;

bool is_running;
} // Anonymous namespace

/**
 * Cache the JavaVM so that we can call into it later.
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_java_vm = vm;

    return JNI_VERSION_1_6;
}

static int RunCitra(const std::string& path) {
    LOG_INFO(Frontend, "Citra is Starting");
    Config config;
    int option_index = 0;
    bool use_gdbstub = Settings::values.use_gdbstub;
    u32 gdb_port = static_cast<u32>(Settings::values.gdbstub_port);
    std::string movie_record;
    std::string movie_play;

    std::string filepath;

    filepath = path;

    Log::Filter log_filter;
    log_filter.ParseFilterString(Settings::values.log_filter);
    Log::SetGlobalFilter(log_filter);

    Log::AddBackend(std::make_unique<Log::ColorConsoleBackend>());
    FileUtil::CreateFullPath(FileUtil::GetUserPath(D_LOGS_IDX));
    Log::AddBackend(
        std::make_unique<Log::FileBackend>(FileUtil::GetUserPath(D_LOGS_IDX) + LOG_FILE));
    MicroProfileOnThreadCreate("EmuThread");
    SCOPE_EXIT({ MicroProfileShutdown(); });

    if (filepath.empty()) {
        LOG_CRITICAL(Frontend, "Failed to load ROM: No ROM specified");
        return -1;
    }

    if (!movie_record.empty() && !movie_play.empty()) {
        LOG_CRITICAL(Frontend, "Cannot both play and record a movie");
        return -1;
    }

    log_filter.ParseFilterString(Settings::values.log_filter);

    // Register frontend applets
    Frontend::RegisterDefaultApplets();

    // Apply the command line arguments
    Settings::values.gdbstub_port = gdb_port;
    Settings::values.use_gdbstub = use_gdbstub;
    Settings::Apply();

    InputManager::Init();
    emu = new EmuWindow_Android(s_surf);
    Core::System& system{Core::System::GetInstance()};

    SCOPE_EXIT({
        system.Shutdown();
        InputManager::Shutdown();
        emu->~EmuWindow_Android();
    });

    const Core::System::ResultStatus load_result{system.Load(*emu, filepath)};

    switch (load_result) {
    case Core::System::ResultStatus::ErrorGetLoader:
        LOG_CRITICAL(Frontend, "Failed to obtain loader for {}!", filepath);
        return -1;
    case Core::System::ResultStatus::ErrorLoader:
        LOG_CRITICAL(Frontend, "Failed to load ROM!");
        return -1;
    case Core::System::ResultStatus::ErrorLoader_ErrorEncrypted:
        LOG_CRITICAL(Frontend, "The game that you are trying to load must be decrypted before "
                               "being used with Citra. \n\n For more information on dumping and "
                               "decrypting games, please refer to: "
                               "https://citra-emu.org/wiki/dumping-game-cartridges/");
        return -1;
    case Core::System::ResultStatus::ErrorLoader_ErrorInvalidFormat:
        LOG_CRITICAL(Frontend, "Error while loading ROM: The ROM format is not supported.");
        return -1;
    case Core::System::ResultStatus::ErrorNotInitialized:
        LOG_CRITICAL(Frontend, "CPUCore not initialized");
        return -1;
    case Core::System::ResultStatus::ErrorSystemMode:
        LOG_CRITICAL(Frontend, "Failed to determine system mode!");
        return -1;
    case Core::System::ResultStatus::ErrorVideoCore:
        LOG_CRITICAL(Frontend, "VideoCore not initialized");
        return -1;
    case Core::System::ResultStatus::Success:
        break; // Expected case
    }

    Core::Telemetry().AddField(Telemetry::FieldType::App, "Frontend", "SDL");

    if (!movie_play.empty()) {
        Core::Movie::GetInstance().StartPlayback(movie_play);
    }
    if (!movie_record.empty()) {
        Core::Movie::GetInstance().StartRecording(movie_record);
    }

    while (is_running) {
        system.RunLoop();
    }

    return 0;
}

static std::string GetJString(JNIEnv* env, jstring jstr) {
    std::string result = "";
    if (!jstr)
        return result;

    const char* s = env->GetStringUTFChars(jstr, nullptr);
    result = s;
    env->ReleaseStringUTFChars(jstr, s);
    return result;
}

void Java_org_citra_citra_1android_NativeLibrary_SurfaceChanged(JNIEnv* env, jobject obj,
                                                                jobject surf) {
    s_surf = ANativeWindow_fromSurface(env, surf);

    if (s_surf == nullptr)
        LOG_ERROR(Frontend, "Error: Surface is null.");
    else if (is_running) {
        emu->OnSurfaceChanged(s_surf);
    }

    LOG_INFO(Frontend, "Surface changed");
}

void Java_org_citra_citra_1android_NativeLibrary_SurfaceDestroyed(JNIEnv* env, jobject obj) {
    // is_running = false;
}

void Java_org_citra_citra_1android_NativeLibrary_CacheClassesAndMethods(JNIEnv* env, jobject obj) {
    // This class reference is only valid for the lifetime of this method.
    jclass localClass = env->FindClass("org/citra/citra_android/NativeLibrary");

    // This reference, however, is valid until we delete it.
    s_jni_class = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));

    // TODO Find a place for this.
    // So we don't leak a reference to NativeLibrary.class.
    // env->DeleteGlobalRef(s_jni_class);

    // Method signature taken from javap -s
    // Source/Android/app/build/intermediates/classes/arm/debug/org/dolphinemu/dolphinemu/NativeLibrary.class
    s_jni_method_alert = env->GetStaticMethodID(s_jni_class, "displayAlertMsg",
                                                "(Ljava/lang/String;Ljava/lang/String;Z)Z");
}

void Java_org_citra_citra_1android_NativeLibrary_SetUserDirectory(JNIEnv* env, jobject obj,
                                                                  jstring jDirectory) {
    FileUtil::SetCurrentDir(GetJString(env, jDirectory));
}

void Java_org_citra_citra_1android_NativeLibrary_UnPauseEmulation(JNIEnv* env, jobject obj) {}

void Java_org_citra_citra_1android_NativeLibrary_PauseEmulation(JNIEnv* env, jobject obj) {}

void Java_org_citra_citra_1android_NativeLibrary_StopEmulation(JNIEnv* env, jobject obj) {
    bool is_running = false;
}

jboolean Java_org_citra_citra_1android_NativeLibrary_IsRunning(JNIEnv* env, jobject obj) {
    return static_cast<jboolean>(is_running);
}

jboolean Java_org_citra_citra_1android_NativeLibrary_onGamePadEvent(JNIEnv* env, jobject obj,
                                                                    jstring jDevice, jint button,
                                                                    jint pressed) {
    if (pressed) {
        InputManager::ButtonHandler()->PressKey(button);
    } else {
        InputManager::ButtonHandler()->ReleaseKey(button);
    }

    return static_cast<jboolean>(true);
}

void Java_org_citra_citra_1android_NativeLibrary_onGamePadMoveEvent(JNIEnv* env, jobject obj,
                                                                    jstring jDevice, jint Axis,
                                                                    jfloat x, jfloat y) {
    // Citra uses an inverted y axis sent by the frontend
    y = -y;
    InputManager::AnalogHandler()->MoveJoystick(Axis, x, y);
}

void Java_org_citra_citra_1android_NativeLibrary_onTouchEvent(JNIEnv* env, jobject obj, jfloat x,
                                                              jfloat y, jboolean pressed) {
    LOG_DEBUG(Frontend, "Touch at x: %d y: %d", (int)x, (int)y);
    emu->OnTouchEvent((int)x, (int)y, (bool)pressed);
}

void Java_org_citra_citra_1android_NativeLibrary_onTouchMoved(JNIEnv* env, jobject obj, jfloat x,
                                                              jfloat y) {
    LOG_DEBUG(Frontend, "Touch at x: %d y: %d", (int)x, (int)y);
    emu->OnTouchMoved((int)x, (int)y);
}

jintArray Java_org_citra_citra_1android_NativeLibrary_GetBanner(JNIEnv* env, jobject obj,
                                                                jstring jFilepath) {
    std::string filepath = GetJString(env, jFilepath);

    std::vector<u16> icon_data = GameInfo::GetIcon(filepath);
    if (icon_data.size() == 0) {
        return 0;
    }

    jintArray Banner = env->NewIntArray(icon_data.size());
    env->SetIntArrayRegion(Banner, 0, icon_data.size(), reinterpret_cast<jint*>(icon_data.data()));

    return Banner;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetTitle(JNIEnv* env, jobject obj,
                                                             jstring jFilepath) {
    std::string filepath = GetJString(env, jFilepath);

    char16_t* Title = GameInfo::GetTitle(filepath);

    if (!Title) {
        return env->NewStringUTF("");
    }

    return env->NewStringUTF(Common::UTF16ToUTF8(Title).data());
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetDescription(JNIEnv* env, jobject obj,
                                                                   jstring jFilename) {
    return jFilename;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetGameId(JNIEnv* env, jobject obj,
                                                              jstring jFilename) {
    return jFilename;
}

jint Java_org_citra_citra_1android_NativeLibrary_GetCountry(JNIEnv* env, jobject obj,
                                                            jstring jFilename) {
    return 0;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetCompany(JNIEnv* env, jobject obj,
                                                               jstring jFilepath) {
    std::string filepath = GetJString(env, jFilepath);

    char16_t* Publisher = GameInfo::GetPublisher(filepath);

    if (!Publisher) {
        return nullptr;
    }

    return env->NewStringUTF(Common::UTF16ToUTF8(Publisher).data());
}

jlong Java_org_citra_citra_1android_NativeLibrary_GetFilesize(JNIEnv* env, jobject obj,
                                                              jstring jFilename) {
    return 0;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetVersionString(JNIEnv* env, jobject obj) {
    return nullptr;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetGitRevision(JNIEnv* env, jobject obj) {
    return nullptr;
}

void Java_org_citra_citra_1android_NativeLibrary_SaveScreenShot(JNIEnv* env, jobject obj) {}

void Java_org_citra_citra_1android_NativeLibrary_eglBindAPI(JNIEnv* env, jobject obj, jint api) {}

jstring Java_org_citra_citra_1android_NativeLibrary_GetConfig(JNIEnv* env, jobject obj,
                                                              jstring jFile, jstring jSection,
                                                              jstring jKey, jstring jDefault) {
    return nullptr;
}

void Java_org_citra_citra_1android_NativeLibrary_SetConfig(JNIEnv* env, jobject obj, jstring jFile,
                                                           jstring jSection, jstring jKey,
                                                           jstring jValue) {}

void Java_org_citra_citra_1android_NativeLibrary_SetFilename(JNIEnv* env, jobject obj,
                                                             jstring jFile) {}

void Java_org_citra_citra_1android_NativeLibrary_SaveState(JNIEnv* env, jobject obj, jint slot,
                                                           jboolean wait) {}

void Java_org_citra_citra_1android_NativeLibrary_SaveStateAs(JNIEnv* env, jobject obj, jstring path,
                                                             jboolean wait) {}

void Java_org_citra_citra_1android_NativeLibrary_LoadState(JNIEnv* env, jobject obj, jint slot) {}

void Java_org_citra_citra_1android_NativeLibrary_LoadStateAs(JNIEnv* env, jobject obj,
                                                             jstring path) {}

void Java_org_citra_citra_1android_services_DirectoryInitializationService_CreateUserDirectories(
    JNIEnv* env, jobject obj) {}

jstring Java_org_citra_citra_1android_NativeLibrary_GetUserDirectory(JNIEnv* env, jobject obj) {

    return nullptr;
}

void Java_org_citra_citra_1android_NativeLibrary_CreateConfigFile() {
    new Config();
}

jint Java_org_citra_citra_1android_NativeLibrary_DefaultCPUCore(JNIEnv* env, jobject obj) {
    return 0;
}

void Java_org_citra_citra_1android_NativeLibrary_SetProfiling(JNIEnv* env, jobject obj,
                                                              jboolean enable) {}

void Java_org_citra_citra_1android_NativeLibrary_WriteProfileResults(JNIEnv* env, jobject obj) {}

void Java_org_citra_citra_1android_NativeLibrary_Run__Ljava_lang_String_2Ljava_lang_String_2Z(
    JNIEnv* env, jobject obj, jstring jFile, jstring jSavestate, jboolean jDeleteSavestate) {}

jstring Java_org_citra_citra_1android_NativeLibrary_GetUserSetting(JNIEnv* env, jclass type,
                                                                   jstring gameID_,
                                                                   jstring Section_, jstring Key_) {
    const char* gameID = env->GetStringUTFChars(gameID_, 0);
    const char* Section = env->GetStringUTFChars(Section_, 0);
    const char* Key = env->GetStringUTFChars(Key_, 0);

    // TODO

    env->ReleaseStringUTFChars(gameID_, gameID);
    env->ReleaseStringUTFChars(Section_, Section);
    env->ReleaseStringUTFChars(Key_, Key);

    return env->NewStringUTF("");
}

void Java_org_citra_citra_1android_NativeLibrary_SetUserSetting(JNIEnv* env, jclass type,
                                                                jstring gameID_, jstring Section_,
                                                                jstring Key_, jstring Value_) {
    const char* gameID = env->GetStringUTFChars(gameID_, 0);
    const char* Section = env->GetStringUTFChars(Section_, 0);
    const char* Key = env->GetStringUTFChars(Key_, 0);
    const char* Value = env->GetStringUTFChars(Value_, 0);

    // TODO

    env->ReleaseStringUTFChars(gameID_, gameID);
    env->ReleaseStringUTFChars(Section_, Section);
    env->ReleaseStringUTFChars(Key_, Key);
    env->ReleaseStringUTFChars(Value_, Value);
}

void Java_org_citra_citra_1android_NativeLibrary_InitGameIni(JNIEnv* env, jclass type,
                                                             jstring gameID_) {
    const char* gameID = env->GetStringUTFChars(gameID_, 0);

    // TODO

    env->ReleaseStringUTFChars(gameID_, gameID);
}

void Java_org_citra_citra_1android_NativeLibrary_ChangeDisc(JNIEnv* env, jclass type,
                                                            jstring path_) {
    const char* path = env->GetStringUTFChars(path_, 0);

    // TODO

    env->ReleaseStringUTFChars(path_, path);
}

void Java_org_citra_citra_1android_NativeLibrary_RefreshWiimotes(JNIEnv* env, jclass type) {

    // TODO
}

jint Java_org_citra_citra_1android_NativeLibrary_GetPlatform(JNIEnv* env, jclass type,
                                                             jstring filename_) {
    return 0;
}

jdoubleArray Java_org_citra_citra_1android_NativeLibrary_GetPerfStats(JNIEnv* env, jclass type) {

    auto results = Core::System::GetInstance().GetAndResetPerfStats();

    // Converting the structure into an array makes it easier to pass it to the frontend
    double stats[4] = {results.system_fps, results.game_fps, results.frametime,
                       results.emulation_speed};

    jdoubleArray jstats = env->NewDoubleArray(4);
    env->SetDoubleArrayRegion(jstats, 0, 4, stats);

    return jstats;
}

void Java_org_citra_citra_1android_services_DirectoryInitializationService_SetSysDirectory(
    JNIEnv* env, jclass type, jstring path_) {
    const char* path = env->GetStringUTFChars(path_, 0);

    env->ReleaseStringUTFChars(path_, path);
}

void Java_org_citra_citra_1android_NativeLibrary_Run__Ljava_lang_String_2(JNIEnv* env, jclass type,
                                                                          jstring path_) {
    const std::string path = GetJString(env, path_);

    is_running = true;
    RunCitra(path);
}