#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#include <android/native_window_jni.h>
#include <jni.h>

#include "citra/config.h"
#include "common/file_util.h"
#include "common/logging/backend.h"
#include "common/logging/filter.h"
#include "common/logging/log.h"
#include "common/microprofile.h"
#include "common/scm_rev.h"
#include "common/scope_exit.h"
#include "common/string_util.h"
#include "core/core.h"
#include "core/file_sys/cia_container.h"
#include "core/gdbstub/gdbstub.h"
#include "core/hle/service/am/am.h"
#include "core/hle/service/fs/archive.h"
#include "core/loader/loader.h"
#include "core/loader/smdh.h"
#include "core/settings.h"
#include "emu_window/emu_window.h"
#include "input_common/keyboard.h"
#include "input_common/main.h"
#include "network/network.h"
#include "native.h"
#include "button_manager.h"

JavaVM *g_java_vm;

namespace {
    ANativeWindow *s_surf;

    jclass s_jni_class;
    jmethodID s_jni_method_alert;

    EmuWindow_Android *emu;

    bool is_running;
}  // Anonymous namespace

/**
 * Cache the JavaVM so that we can call into it later.
 */
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    g_java_vm = vm;

    return JNI_VERSION_1_6;
}

std::vector<u8> GetSMDHData(std::string physical_name) {
    std::unique_ptr<Loader::AppLoader> loader = Loader::GetLoader(physical_name);
    if (!loader)
        LOG_ERROR(Frontend, "Failed to obtain loader");


    u64 program_id = 0;
    loader->ReadProgramId(program_id);

    std::vector<u8> smdh = [program_id, &loader]() -> std::vector<u8> {
        std::vector<u8> original_smdh;
        loader->ReadIcon(original_smdh);

        if (program_id < 0x00040000'00000000 || program_id > 0x00040000'FFFFFFFF)
            return original_smdh;

        std::string update_path = Service::AM::GetTitleContentPath(
                Service::FS::MediaType::SDMC, program_id + 0x0000000E'00000000);

        if (!FileUtil::Exists(update_path))
            return original_smdh;

        std::unique_ptr<Loader::AppLoader> update_loader = Loader::GetLoader(update_path);

        if (!update_loader)
            return original_smdh;

        std::vector<u8> update_smdh;
        update_loader->ReadIcon(update_smdh);
        return update_smdh;
    }();

    return smdh;
}

static int RunCitra(const std::string &path) {
    LOG_INFO(Frontend, "Citra is Starting");
    Config config;
    int option_index = 0;
    bool use_gdbstub = Settings::values.use_gdbstub;
    u32 gdb_port = static_cast<u32>(Settings::values.gdbstub_port);
    std::string movie_record;
    std::string movie_play;

    std::string filepath;

    filepath = path;

    Log::Filter log_filter(Log::Level::Debug);
    Log::SetFilter(&log_filter);

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

    // Apply the command line arguments
    Settings::values.gdbstub_port = gdb_port;
    Settings::values.use_gdbstub = use_gdbstub;
    Settings::values.movie_play = std::move(movie_play);
    Settings::values.movie_record = std::move(movie_record);
    Settings::Apply();

    InputManager::Init();
    emu = new EmuWindow_Android(s_surf);
    Core::System &system{Core::System::GetInstance()};

    SCOPE_EXIT({
                   system.Shutdown();
                   InputManager::Shutdown();
                   emu->~EmuWindow_Android();
               });

    const Core::System::ResultStatus load_result{system.Load(emu, filepath)};

    switch (load_result) {
        case Core::System::ResultStatus::ErrorGetLoader:
            LOG_CRITICAL(Frontend, "Failed to obtain loader for %s!", filepath.c_str());
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


    do {
        system.RunLoop();

    } while (is_running);

    return 0;
}

static std::string GetJString(JNIEnv *env, jstring jstr) {
    std::string result = "";
    if (!jstr)
        return result;

    const char *s = env->GetStringUTFChars(jstr, nullptr);
    result = s;
    env->ReleaseStringUTFChars(jstr, s);
    return result;
}

void Java_org_citra_citra_1android_NativeLibrary_SurfaceChanged(JNIEnv *env, jobject obj,
                                                                jobject surf) {
    s_surf = ANativeWindow_fromSurface(env, surf);

    if (s_surf == nullptr)
        LOG_ERROR(Frontend, "Error: Surface is null.");
    else if (is_running) {
        emu->OnSurfaceChanged(s_surf);
    }

    LOG_INFO(Frontend, "Surface changed");
}

void Java_org_citra_citra_1android_NativeLibrary_SurfaceDestroyed(JNIEnv *env, jobject obj) {
    //is_running = false;
}

void
Java_org_citra_citra_1android_NativeLibrary_CacheClassesAndMethods(JNIEnv *env, jobject obj) {
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


void
Java_org_citra_citra_1android_NativeLibrary_SetUserDirectory(JNIEnv *env, jobject obj,
                                                             jstring jDirectory) {
    FileUtil::SetCurrentDir(GetJString(env, jDirectory));
}

void Java_org_citra_citra_1android_NativeLibrary_UnPauseEmulation(JNIEnv *env, jobject obj) {

}

void Java_org_citra_citra_1android_NativeLibrary_PauseEmulation(JNIEnv *env, jobject obj) {

}

void Java_org_citra_citra_1android_NativeLibrary_StopEmulation(JNIEnv *env, jobject obj) {

}

jboolean Java_org_citra_citra_1android_NativeLibrary_IsRunning(JNIEnv *env, jobject obj) {
    return static_cast<jboolean>(true);
}

jboolean Java_org_citra_citra_1android_NativeLibrary_onGamePadEvent(JNIEnv *env, jobject obj,
                                                                    jstring jDevice, jint button,
                                                                    jint pressed) {
    if (pressed) {
        InputManager::ButtonHandler()->PressKey(button);
    }
    else {
        InputManager::ButtonHandler()->ReleaseKey(button);
    }

    return static_cast<jboolean>(true);
}

void Java_org_citra_citra_1android_NativeLibrary_onGamePadMoveEvent(JNIEnv *env, jobject obj,
                                                                    jstring jDevice, jint Axis,
                                                                    jfloat x, jfloat y) {
    // Citra uses an inverted y axis sent by the frontend
    y =-y;
    InputManager::AnalogHandler()->MoveJoystick(Axis, x, y);
}

jintArray Java_org_citra_citra_1android_NativeLibrary_GetBanner(JNIEnv *env, jobject obj,
                                                                jstring jFilepath) {
    int size = 48;

    std::string filepath = GetJString(env, jFilepath);
    std::vector<u8> smdh_data = GetSMDHData(filepath);

    if (!Loader::IsValidSMDH(smdh_data)) {
        // SMDH is not valid, set a default icon
        LOG_ERROR(Frontend, "SMDH is Invalid");
        return 0;
    }

    Loader::SMDH smdh;
    memcpy(&smdh, smdh_data.data(), sizeof(Loader::SMDH));

    // Always get a 48x48(large) icon
    std::vector<u16> icon_data = smdh.GetIcon(true);

    jintArray Banner = env->NewIntArray(size * size);
    env->SetIntArrayRegion(Banner, 0, size * size, reinterpret_cast<jint *>(icon_data.data()));

    return Banner;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetTitle(JNIEnv *env, jobject obj,
                                                             jstring jFilepath) {
    Loader::SMDH::TitleLanguage language = Loader::SMDH::TitleLanguage::English;
    std::string filepath = GetJString(env, jFilepath);
    std::vector<u8> smdh_data = GetSMDHData(filepath);


    if (!Loader::IsValidSMDH(smdh_data)) {
        // SMDH is not valid, Return the file name;
        LOG_ERROR(Frontend, "SMDH is Invalid");
        return jFilepath;
    }

    Loader::SMDH smdh;
    memcpy(&smdh, smdh_data.data(), sizeof(Loader::SMDH));

    // Get the title from SMDH in UTF-16 format
    char16_t *Title;
    Title = reinterpret_cast<char16_t *>(smdh.titles[static_cast<int>(language)]
            .long_title.data());


    LOG_INFO(Frontend, "Title: %s", Common::UTF16ToUTF8(Title).data());

    return env->NewStringUTF(Common::UTF16ToUTF8(Title).data());
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetDescription(JNIEnv *env, jobject obj,
                                                                   jstring jFilename) {
    return jFilename;
}

jstring
Java_org_citra_citra_1android_NativeLibrary_GetGameId(JNIEnv *env, jobject obj, jstring jFilename) {
    return jFilename;
}

jint Java_org_citra_citra_1android_NativeLibrary_GetCountry(JNIEnv *env, jobject obj,
                                                            jstring jFilename) {
    return 0;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetCompany(JNIEnv *env, jobject obj,
                                                               jstring jFilepath) {
    Loader::SMDH::TitleLanguage language = Loader::SMDH::TitleLanguage::English;
    std::string filepath = GetJString(env, jFilepath);
    std::vector<u8> smdh_data = GetSMDHData(filepath);


    if (!Loader::IsValidSMDH(smdh_data)) {
        // SMDH is not valid ,return null
        LOG_ERROR(Frontend, "SMDH is Invalid");
        return nullptr;
    }

    Loader::SMDH smdh;
    memcpy(&smdh, smdh_data.data(), sizeof(Loader::SMDH));

    // Get the Publisher's name from SMDH in UTF-16 format
    char16_t *Publisher;
    Publisher = reinterpret_cast<char16_t *>(smdh.titles[static_cast<int>(language)]
            .publisher.data());


    LOG_INFO(Frontend, "Publisher: %s", Common::UTF16ToUTF8(Publisher).data());

    return env->NewStringUTF(Common::UTF16ToUTF8(Publisher).data());
}

jlong Java_org_citra_citra_1android_NativeLibrary_GetFilesize(JNIEnv *env, jobject obj,
                                                              jstring jFilename) {
    return 0;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetVersionString(JNIEnv *env, jobject obj) {
    return nullptr;
}

jstring Java_org_citra_citra_1android_NativeLibrary_GetGitRevision(JNIEnv *env, jobject obj) {
    return nullptr;
}

void Java_org_citra_citra_1android_NativeLibrary_SaveScreenShot(JNIEnv *env, jobject obj) {

}

void Java_org_citra_citra_1android_NativeLibrary_eglBindAPI(JNIEnv *env, jobject obj, jint api) {

}

jstring Java_org_citra_citra_1android_NativeLibrary_GetConfig(JNIEnv *env, jobject obj,
                                                              jstring jFile, jstring jSection,
                                                              jstring jKey, jstring jDefault) {
    return nullptr;
}

void Java_org_citra_citra_1android_NativeLibrary_SetConfig(JNIEnv *env, jobject obj, jstring jFile,
                                                           jstring jSection, jstring jKey,
                                                           jstring jValue) {

}

void
Java_org_citra_citra_1android_NativeLibrary_SetFilename(JNIEnv *env, jobject obj, jstring jFile) {

}

void Java_org_citra_citra_1android_NativeLibrary_SaveState(JNIEnv *env, jobject obj, jint slot,
                                                           jboolean wait) {

}

void Java_org_citra_citra_1android_NativeLibrary_SaveStateAs(JNIEnv *env, jobject obj, jstring path,
                                                             jboolean wait) {

}

void Java_org_citra_citra_1android_NativeLibrary_LoadState(JNIEnv *env, jobject obj, jint slot) {

}

void
Java_org_citra_citra_1android_NativeLibrary_LoadStateAs(JNIEnv *env, jobject obj, jstring path) {

}

void Java_org_citra_citra_1android_services_DirectoryInitializationService_CreateUserDirectories(
        JNIEnv *env, jobject obj) {

}

jstring Java_org_citra_citra_1android_NativeLibrary_GetUserDirectory(JNIEnv *env, jobject obj) {
    return nullptr;
}

jint Java_org_citra_citra_1android_NativeLibrary_DefaultCPUCore(JNIEnv *env, jobject obj) {
    return 0;
}

void Java_org_citra_citra_1android_NativeLibrary_SetProfiling(JNIEnv *env, jobject obj,
                                                              jboolean enable) {

}

void Java_org_citra_citra_1android_NativeLibrary_WriteProfileResults(JNIEnv *env, jobject obj) {

}

void Java_org_citra_citra_1android_NativeLibrary_Run__Ljava_lang_String_2Ljava_lang_String_2Z(
        JNIEnv *env, jobject obj, jstring jFile, jstring jSavestate, jboolean jDeleteSavestate) {

}

jstring Java_org_citra_citra_1android_NativeLibrary_GetUserSetting(JNIEnv *env, jclass type,
                                                                   jstring gameID_,
                                                                   jstring Section_,
                                                                   jstring Key_) {
    const char *gameID = env->GetStringUTFChars(gameID_, 0);
    const char *Section = env->GetStringUTFChars(Section_, 0);
    const char *Key = env->GetStringUTFChars(Key_, 0);

    // TODO

    env->ReleaseStringUTFChars(gameID_, gameID);
    env->ReleaseStringUTFChars(Section_, Section);
    env->ReleaseStringUTFChars(Key_, Key);

    return env->NewStringUTF("");
}

void Java_org_citra_citra_1android_NativeLibrary_SetUserSetting(JNIEnv *env, jclass type,
                                                                jstring gameID_, jstring Section_,
                                                                jstring Key_, jstring Value_) {
    const char *gameID = env->GetStringUTFChars(gameID_, 0);
    const char *Section = env->GetStringUTFChars(Section_, 0);
    const char *Key = env->GetStringUTFChars(Key_, 0);
    const char *Value = env->GetStringUTFChars(Value_, 0);

    // TODO

    env->ReleaseStringUTFChars(gameID_, gameID);
    env->ReleaseStringUTFChars(Section_, Section);
    env->ReleaseStringUTFChars(Key_, Key);
    env->ReleaseStringUTFChars(Value_, Value);
}

void
Java_org_citra_citra_1android_NativeLibrary_InitGameIni(JNIEnv *env, jclass type, jstring gameID_) {
    const char *gameID = env->GetStringUTFChars(gameID_, 0);

    // TODO

    env->ReleaseStringUTFChars(gameID_, gameID);
}

void
Java_org_citra_citra_1android_NativeLibrary_ChangeDisc(JNIEnv *env, jclass type, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    // TODO

    env->ReleaseStringUTFChars(path_, path);
}

void Java_org_citra_citra_1android_NativeLibrary_RefreshWiimotes(JNIEnv *env, jclass type) {

    // TODO

}

jint Java_org_citra_citra_1android_NativeLibrary_GetPlatform(JNIEnv *env, jclass type,
                                                             jstring filename_) {

    // Return 1 and let the frontend think the game is a wii game,
    // this lets us use the all the controllers
    return 1;
}

void Java_org_citra_citra_1android_services_DirectoryInitializationService_SetSysDirectory(
                                                                                      JNIEnv *env,
                                                                                      jclass type,
                                                                                      jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    env->ReleaseStringUTFChars(path_, path);
}

void
Java_org_citra_citra_1android_NativeLibrary_Run__Ljava_lang_String_2(JNIEnv *env, jclass type,
                                                                     jstring path_) {
    const std::string path = GetJString(env, path_);

    is_running = true;
    RunCitra(path);


}