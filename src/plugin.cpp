#include "Settings.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
//  ShoutAimHandler
//
//  Listens for animation graph events on the player character.
//  When "BeginCastVoice" fires (the start of a shout animation, BEFORE the
//  voice spell effect is launched), the handler rotates the player to face
//  the camera/crosshair direction — so the shout aims where the player is
//  looking, not where the character model happens to face.
// ─────────────────────────────────────────────────────────────────────────────
class ShoutAimHandler final : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
    // ── Singleton ────────────────────────────────────────────────────────
    static ShoutAimHandler* GetSingleton()
    {
        static ShoutAimHandler instance;
        return &instance;
    }

    ShoutAimHandler(const ShoutAimHandler&)            = delete;
    ShoutAimHandler(ShoutAimHandler&&)                 = delete;
    ShoutAimHandler& operator=(const ShoutAimHandler&) = delete;
    ShoutAimHandler& operator=(ShoutAimHandler&&)      = delete;

    // ── Event handler ────────────────────────────────────────────────────
    RE::BSEventNotifyControl ProcessEvent(
        const RE::BSAnimationGraphEvent* a_event,
        [[maybe_unused]] RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_source) override
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const auto* settings = Settings::GetSingleton();
        if (!settings->enable) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // ── Only process the shout-start animation event ─────────────
        // "BeginCastVoice" fires at the beginning of the shout animation,
        // before "Voice_SpellFire_Event" which actually launches the effect.
        const auto& tag = a_event->tag;
        if (tag != "BeginCastVoice") {
            return RE::BSEventNotifyControl::kContinue;
        }

        // ── Verify this event is for the player ──────────────────────
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // The event's holder should be the player since we registered the
        // sink on the player's animation graph.  Extra safety check:
        auto* holder = a_event->holder;
        if (!holder || holder != player) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // ── Check weapon state ───────────────────────────────────────
        if (settings->onlyWhenWeaponsSheathed && player->IsWeaponDrawn()) {
            if (settings->debugLogging) {
                logger::debug("[ShoutAim] Shout detected but weapons are drawn — skipping.");
            }
            return RE::BSEventNotifyControl::kContinue;
        }

        // ── Apply aim correction ─────────────────────────────────────
        CorrectShoutAim(player);

        return RE::BSEventNotifyControl::kContinue;
    }

    // ── Registration ─────────────────────────────────────────────────────
    /// Register the event sink on the player's animation graph.
    /// Must be called AFTER the player is fully loaded (e.g. kDataLoaded).
    static void Register()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            logger::error("[ShoutAim] PlayerCharacter singleton not available — cannot register.");
            return;
        }

        player->AddAnimationGraphEventSink(GetSingleton());
        logger::info("[ShoutAim] Animation graph event sink registered on player.");
    }

private:
    ShoutAimHandler() = default;
    ~ShoutAimHandler() override = default;

    // ── Camera yaw extraction ────────────────────────────────────────────
    /// Get the camera's horizontal yaw angle (radians) from the camera
    /// node's world rotation matrix.
    ///
    /// This reads the ACTUAL rendered camera direction, which is compatible
    /// with True Directional Movement, SmoothCam, and other camera mods.
    ///
    /// Skyrim coordinate system:
    ///   X = East,  Y = North,  Z = Up
    /// Gamebryo/NetImmerse cameras look along local -Z,
    ///   so forward = -(column 2 of the rotation matrix).
    /// Yaw = atan2(East component, North component).
    static float GetCameraYaw()
    {
        auto* camera = RE::PlayerCamera::GetSingleton();
        if (!camera || !camera->cameraRoot) {
            return 0.0f;
        }

        const auto& rot = camera->cameraRoot->world.rotate;

        // Forward direction = negated third column of the NiMatrix3
        const float forwardX = -rot.entry[0][2];  // East component
        const float forwardY = -rot.entry[1][2];  // North component

        return std::atan2(forwardX, forwardY);
    }

    /// Get the camera's vertical pitch angle (radians).
    /// Positive = looking up, negative = looking down.
    static float GetCameraPitch()
    {
        auto* camera = RE::PlayerCamera::GetSingleton();
        if (!camera || !camera->cameraRoot) {
            return 0.0f;
        }

        const auto& rot = camera->cameraRoot->world.rotate;

        // Forward Z component gives the vertical angle
        const float forwardZ = -rot.entry[2][2];

        // Clamp to avoid NaN from asin
        const float clamped = std::clamp(forwardZ, -1.0f, 1.0f);
        return std::asin(clamped);
    }

    // ── Aim correction ───────────────────────────────────────────────────
    static void CorrectShoutAim(RE::PlayerCharacter* a_player)
    {
        const auto* settings = Settings::GetSingleton();

        if (settings->matchCameraYaw) {
            const float cameraYaw = GetCameraYaw();
            const float prevYaw   = a_player->data.angle.z;

            // Set the player's facing direction to match the camera
            a_player->SetRotationZ(cameraYaw);

            if (settings->debugLogging) {
                logger::debug(
                    "[ShoutAim] Corrected yaw: {:.4f} rad -> {:.4f} rad "
                    "(delta {:.4f} rad / {:.1f} deg)",
                    prevYaw, cameraYaw,
                    cameraYaw - prevYaw,
                    (cameraYaw - prevYaw) * 180.0f / 3.14159265f);
            }
        }

        // matchCameraPitch is reserved for future implementation.
        // The vanilla engine handles shout pitch through its own mechanisms,
        // and overriding it risks breaking cone-shaped shouts (Unrelenting Force)
        // or area shouts (Storm Call).  Only enable this if you understand
        // the projectile-type implications.
        if (settings->matchCameraPitch) {
            const float cameraPitch = GetCameraPitch();
            const float prevPitch   = a_player->data.angle.x;

            a_player->data.angle.x = cameraPitch;

            if (settings->debugLogging) {
                logger::debug(
                    "[ShoutAim] Corrected pitch: {:.4f} rad -> {:.4f} rad",
                    prevPitch, cameraPitch);
            }
        }
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  SKSE messaging callback
// ─────────────────────────────────────────────────────────────────────────────
static void MessageCallback(SKSE::MessagingInterface::Message* a_msg)
{
    if (!a_msg) {
        return;
    }

    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        // Game data is fully loaded — safe to access singletons.
        logger::info("[ShoutAim] kDataLoaded received — registering handler.");
        ShoutAimHandler::Register();
        break;

    case SKSE::MessagingInterface::kNewGame:
        // Starting a new game — re-register in case internals were reset.
        logger::info("[ShoutAim] kNewGame — re-registering handler.");
        ShoutAimHandler::Register();
        break;

    case SKSE::MessagingInterface::kPostLoadGame:
        // Loading a save — re-register to ensure the sink is active.
        logger::info("[ShoutAim] kPostLoadGame — re-registering handler.");
        ShoutAimHandler::Register();
        break;

    default:
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  SKSE plugin entry point
// ─────────────────────────────────────────────────────────────────────────────

/// Called by SKSE to query plugin info.  CommonLibSSE-NG handles this
/// automatically via add_commonlibsse_plugin, but we declare it for
/// explicit version info if needed.
SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    // ── Logging ──────────────────────────────────────────────────────
    auto* log = SKSE::log::get();
    if (log) {
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
    }

    logger::info("{} v{} loaded", Plugin::NAME, Plugin::VERSION.string());
    logger::info("Runtime: {}", REL::Module::get().version().string());

    // ── Settings ─────────────────────────────────────────────────────
    Settings::GetSingleton()->Load();

    // After loading settings, adjust log level if debug logging is enabled
    if (Settings::GetSingleton()->debugLogging && log) {
        log->set_level(spdlog::level::debug);
        log->flush_on(spdlog::level::debug);
        logger::debug("[ShoutAim] Debug logging enabled.");
    }

    // ── Register for SKSE messages ───────────────────────────────────
    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging) {
        logger::critical("[ShoutAim] Failed to get SKSE messaging interface!");
        return false;
    }

    if (!messaging->RegisterListener(MessageCallback)) {
        logger::critical("[ShoutAim] Failed to register messaging listener!");
        return false;
    }

    logger::info("[ShoutAim] Messaging listener registered. Waiting for kDataLoaded...");
    return true;
}
