#pragma once

#include <string_view>

namespace Plugin
{
    inline constexpr std::string_view NAME = "SheathedShoutsAimAtCrosshair";
    inline constexpr std::string_view VERSION = "1.0.0";
}

/// @brief Singleton that holds all user-configurable settings loaded from the INI file.
struct Settings
{
    /// @brief Whether the plugin is active at all.
    bool enable = true;

    /// @brief Only correct aim when weapons are sheathed.
    ///        If false, the correction applies to all shouts regardless of weapon state.
    bool onlyWhenWeaponsSheathed = true;

    /// @brief Rotate the player to match the camera's horizontal (yaw) direction.
    bool matchCameraYaw = true;

    /// @brief (Reserved for future use) Adjust vertical pitch to match the camera.
    ///        Not implemented in the MVP — the engine's vanilla pitch behavior is used.
    bool matchCameraPitch = false;

    /// @brief Emit detailed debug messages to the SKSE log on every aim correction.
    bool debugLogging = false;

    /// @brief Access the global Settings instance.
    static Settings* GetSingleton();

    /// @brief Load settings from the INI file.
    ///        Missing keys silently fall back to the default values defined above.
    void Load();
};
