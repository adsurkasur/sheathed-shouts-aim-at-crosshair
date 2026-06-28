#include "Settings.h"

#include <SimpleIni.h>
#include <SKSE/SKSE.h>

namespace
{
    /// Resolve the path to the INI file next to the plugin DLL.
    /// SKSE log directory: Data/SKSE/Plugins/ — we place our INI there.
    std::filesystem::path GetINIPath()
    {
        auto path = std::filesystem::path{
            fmt::format("Data/SKSE/Plugins/{}.ini",
                        Plugin::NAME)};
        return path;
    }
}

Settings* Settings::GetSingleton()
{
    static Settings instance;
    return &instance;
}

void Settings::Load()
{
    const auto path = GetINIPath();
    const auto pathStr = path.string();

    logger::info("Loading settings from: {}", pathStr);

    CSimpleIniA ini;
    ini.SetUnicode();

    const SI_Error rc = ini.LoadFile(pathStr.c_str());
    if (rc < 0) {
        logger::warn("INI file not found or unreadable ({}). Using defaults.", pathStr);
        return;
    }

    // ── [General] section ───────────────────────────────────────────────
    enable                  = ini.GetBoolValue("General", "Enable", enable);
    onlyWhenWeaponsSheathed = ini.GetBoolValue("General", "OnlyWhenWeaponsSheathed",
                                                onlyWhenWeaponsSheathed);
    matchCameraYaw          = ini.GetBoolValue("General", "MatchCameraYaw", matchCameraYaw);
    matchCameraPitch        = ini.GetBoolValue("General", "MatchCameraPitch", matchCameraPitch);
    debugLogging            = ini.GetBoolValue("General", "DebugLogging", debugLogging);

    // ── Log loaded values ───────────────────────────────────────────────
    logger::info("Settings loaded:");
    logger::info("  Enable                  = {}", enable);
    logger::info("  OnlyWhenWeaponsSheathed = {}", onlyWhenWeaponsSheathed);
    logger::info("  MatchCameraYaw          = {}", matchCameraYaw);
    logger::info("  MatchCameraPitch        = {} (reserved, not implemented)", matchCameraPitch);
    logger::info("  DebugLogging            = {}", debugLogging);
}
