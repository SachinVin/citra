// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstring>
#include <memory>
#include <vector>

#include "citra_android/jni/game_info.h"
#include "common/string_util.h"
#include "core/hle/service/am/am.h"
#include "core/hle/service/fs/archive.h"
#include "core/loader/loader.h"
#include "core/loader/smdh.h"

namespace GameInfo {
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

char16_t* GetTitle(std::string physical_name) {
    Loader::SMDH::TitleLanguage language = Loader::SMDH::TitleLanguage::English;
    std::vector<u8> smdh_data = GetSMDHData(physical_name);

    if (!Loader::IsValidSMDH(smdh_data)) {
        // SMDH is not valid, Return the file name;
        LOG_ERROR(Frontend, "SMDH is Invalid");
        return nullptr;
    }

    Loader::SMDH smdh;
    memcpy(&smdh, smdh_data.data(), sizeof(Loader::SMDH));

    // Get the title from SMDH in UTF-16 format
    char16_t* title;
    title = reinterpret_cast<char16_t*>(smdh.titles[static_cast<int>(language)].long_title.data());

    LOG_INFO(Frontend, "Title: %s", Common::UTF16ToUTF8(title).data());

    return title;
}

char16_t* GetPublisher(std::string physical_name) {
    Loader::SMDH::TitleLanguage language = Loader::SMDH::TitleLanguage::English;
    std::vector<u8> smdh_data = GetSMDHData(physical_name);

    if (!Loader::IsValidSMDH(smdh_data)) {
        // SMDH is not valid, return null
        LOG_ERROR(Frontend, "SMDH is Invalid");
        return nullptr;
    }

    Loader::SMDH smdh;
    memcpy(&smdh, smdh_data.data(), sizeof(Loader::SMDH));

    // Get the Publisher's name from SMDH in UTF-16 format
    char16_t* publisher;
    publisher =
        reinterpret_cast<char16_t*>(smdh.titles[static_cast<int>(language)].publisher.data());

    LOG_INFO(Frontend, "Publisher: %s", Common::UTF16ToUTF8(publisher).data());

    return publisher;
}

std::vector<u16> GetIcon(std::string physical_name) {
    std::vector<u8> smdh_data = GetSMDHData(physical_name);

    if (!Loader::IsValidSMDH(smdh_data)) {
        // SMDH is not valid, return null
        LOG_ERROR(Frontend, "SMDH is Invalid");
        return std::vector<u16>(0, 0);
    }

    Loader::SMDH smdh;
    memcpy(&smdh, smdh_data.data(), sizeof(Loader::SMDH));

    // Always get a 48x48(large) icon
    std::vector<u16> icon_data = smdh.GetIcon(true);
    return icon_data;
}

} // namespace GameInfo