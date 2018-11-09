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

#include "citra_android/jni/native.h"

JavaVM* g_java_vm;

namespace {
ANativeWindow* s_surf;

jclass s_jni_class;
jmethodID s_jni_method_alert;

EmuWindow_Android* emu;

std::atomic<bool> is_running{false};
std::atomic<bool> pause_emulation{false};

std::mutex running_mutex;
std::condition_variable cv;
} // Anonymous namespace

/**
 * Cache the JavaVM so that we can call into it later.
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_java_vm = vm;

    return JNI_VERSION_1_6;
}

static int RunCitra(const std::string& filepath) {
    LOG_INFO(Frontend, "Citra is Starting");
    Config config;
    int option_index = 0;
    bool use_gdbstub = Settings::values.use_gdbstub;
    u32 gdb_port = static_cast<u32>(Settings::values.gdbstub_port);
    std::string movie_record;
    std::string movie_play;

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

    is_running = true;

    while (is_running) {
        if (!pause_emulation) {
            system.RunLoop();
        } else {
            std::unique_lock<std::mutex> lock(running_mutex);
            cv.wait(lock, [] { return !pause_emulation || !is_running; });
        }
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

void JNI_NATIVE_LIBRARY(SurfaceChanged)(JNIEnv* env, jobject obj, jobject surf) {
    s_surf = ANativeWindow_fromSurface(env, surf);

    if (is_running) {
        emu->OnSurfaceChanged(s_surf);
    }

    LOG_INFO(Frontend, "Surface changed");
}

void JNI_NATIVE_LIBRARY(SurfaceDestroyed)(JNIEnv* env, jobject obj) {
    pause_emulation = true;
}

void JNI_NATIVE_LIBRARY(CacheClassesAndMethods)(JNIEnv* env, jobject obj) {
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

void JNI_NATIVE_LIBRARY(SetUserDirectory)(JNIEnv* env, jobject obj, jstring jDirectory) {
    FileUtil::SetCurrentDir(GetJString(env, jDirectory));
}

void JNI_NATIVE_LIBRARY(UnPauseEmulation)(JNIEnv* env, jobject obj) {
    pause_emulation = false;
    cv.notify_all();
}

void JNI_NATIVE_LIBRARY(PauseEmulation)(JNIEnv* env, jobject obj) {}

void JNI_NATIVE_LIBRARY(StopEmulation)(JNIEnv* env, jobject obj) {
    is_running = false;
    cv.notify_all();
}

jboolean JNI_NATIVE_LIBRARY(IsRunning)(JNIEnv* env, jobject obj) {
    return static_cast<jboolean>(is_running);
}

jboolean JNI_NATIVE_LIBRARY(onGamePadEvent)(JNIEnv* env, jobject obj, jstring jDevice, jint button,
                                            jint pressed) {
    bool consumed;
    if (pressed) {
        consumed = InputManager::ButtonHandler()->PressKey(button);
    } else {
        consumed = InputManager::ButtonHandler()->ReleaseKey(button);
    }

    return static_cast<jboolean>(consumed);
}

void JNI_NATIVE_LIBRARY(onGamePadMoveEvent)(JNIEnv* env, jobject obj, jstring jDevice, jint Axis,
                                            jfloat x, jfloat y) {
    // Citra uses an inverted y axis sent by the frontend
    y = -y;
    InputManager::AnalogHandler()->MoveJoystick(Axis, x, y);
}

void JNI_NATIVE_LIBRARY(onTouchEvent)(JNIEnv* env, jobject obj, jfloat x, jfloat y,
                                      jboolean pressed) {
    emu->OnTouchEvent((int)x, (int)y, (bool)pressed);
}

void JNI_NATIVE_LIBRARY(onTouchMoved)(JNIEnv* env, jobject obj, jfloat x, jfloat y) {
    emu->OnTouchMoved((int)x, (int)y);
}

jintArray JNI_NATIVE_LIBRARY(GetBanner)(JNIEnv* env, jobject obj, jstring jFilepath) {
    std::string filepath = GetJString(env, jFilepath);

    std::vector<u16> icon_data = GameInfo::GetIcon(filepath);
    if (icon_data.size() == 0) {
        return 0;
    }

    jintArray icon = env->NewIntArray(icon_data.size());
    env->SetIntArrayRegion(icon, 0, icon_data.size(), reinterpret_cast<jint*>(icon_data.data()));

    return icon;
}

jstring JNI_NATIVE_LIBRARY(GetTitle)(JNIEnv* env, jobject obj, jstring jFilepath) {
    std::string filepath = GetJString(env, jFilepath);

    char16_t* Title = GameInfo::GetTitle(filepath);

    if (!Title) {
        return env->NewStringUTF("");
    }

    return env->NewStringUTF(Common::UTF16ToUTF8(Title).data());
}

jstring JNI_NATIVE_LIBRARY(GetDescription)(JNIEnv* env, jobject obj, jstring jFilename) {
    return jFilename;
}

jstring JNI_NATIVE_LIBRARY(GetGameId)(JNIEnv* env, jobject obj, jstring jFilename) {
    return jFilename;
}

jint JNI_NATIVE_LIBRARY(GetCountry)(JNIEnv* env, jobject obj, jstring jFilename) {
    return 0;
}

jstring JNI_NATIVE_LIBRARY(GetCompany)(JNIEnv* env, jobject obj, jstring jFilepath) {
    std::string filepath = GetJString(env, jFilepath);

    char16_t* Publisher = GameInfo::GetPublisher(filepath);

    if (!Publisher) {
        return nullptr;
    }

    return env->NewStringUTF(Common::UTF16ToUTF8(Publisher).data());
}

jlong JNI_NATIVE_LIBRARY(GetFilesize)(JNIEnv* env, jobject obj, jstring jFilename) {
    return 0;
}

jstring JNI_NATIVE_LIBRARY(GetVersionString)(JNIEnv* env, jobject obj) {
    return nullptr;
}

jstring JNI_NATIVE_LIBRARY(GetGitRevision)(JNIEnv* env, jobject obj) {
    return nullptr;
}

void JNI_NATIVE_LIBRARY(CreateConfigFile)(JNIEnv* env, jobject obj) {
    new Config();
}

void JNI_NATIVE_LIBRARY(SetProfiling)(JNIEnv* env, jobject obj, jboolean enable) {}

void JNI_NATIVE_LIBRARY(WriteProfileResults)(JNIEnv* env, jobject obj) {}

jdoubleArray JNI_NATIVE_LIBRARY(GetPerfStats)(JNIEnv* env, jclass type) {
    auto results = Core::System::GetInstance().GetAndResetPerfStats();
    // Converting the structure into an array makes it easier to pass it to the frontend
    double stats[4] = {results.system_fps, results.game_fps, results.frametime,
                       results.emulation_speed};
    jdoubleArray jstats = env->NewDoubleArray(4);
    env->SetDoubleArrayRegion(jstats, 0, 4, stats);
    return jstats;
}

void JNI_NATIVE_LIBRARY(Run)(JNIEnv* env, jclass type, jstring path_) {
    const std::string path = GetJString(env, path_);

    if (is_running) {
        is_running = false;
    }
    RunCitra(path);
}
