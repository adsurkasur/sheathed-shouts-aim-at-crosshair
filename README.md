# Sheathed Shouts Aim at Crosshair

An SKSE plugin for **Skyrim Special Edition / Anniversary Edition** that makes shouts aim toward the camera/crosshair direction when the player's weapons are sheathed.

In vanilla Skyrim, shouting while weapons are sheathed fires the shout in the direction the player character's body is facing — which is often *not* where the camera is looking, especially with modern third-person setups like True Directional Movement. This plugin fixes that by rotating the player to face the camera direction just before the shout fires.

---

## Requirements

| Dependency | Version | Link |
|---|---|---|
| Skyrim SE / AE | **1.6.1170** (Steam) | — |
| SKSE64 | Compatible with 1.6.1170 | [skse.silverlock.org](https://skse.silverlock.org/) |
| Address Library for SKSE | Latest | [Nexus Mods](https://www.nexusmods.com/skyrimspecialedition/mods/32444) |

---

## Installation

### For Users (Manual)

1. Download the release archive.
2. Copy the contents of the `SKSE/Plugins/` folder to your Skyrim `Data/SKSE/Plugins/` directory:
   ```
   Data/
   └── SKSE/
       └── Plugins/
           ├── SheathedShoutsAimAtCrosshair.dll
           └── SheathedShoutsAimAtCrosshair.ini
   ```
3. Launch the game through SKSE.

### For Users (Mod Manager — MO2 / Vortex)

1. Install the archive as a mod through your preferred mod manager.
2. Enable the mod and launch through SKSE.

---

## INI Settings

The plugin reads settings from `Data/SKSE/Plugins/SheathedShoutsAimAtCrosshair.ini`.  
If the file is missing, all defaults are used.

| Setting | Default | Description |
|---|---|---|
| `Enable` | `1` | Master toggle. Set to `0` to disable the plugin entirely. |
| `OnlyWhenWeaponsSheathed` | `1` | Only correct aim when weapons are sheathed. Set to `0` to always correct. |
| `MatchCameraYaw` | `1` | Rotate player horizontally to match the camera direction. |
| `MatchCameraPitch` | `0` | **(Experimental)** Also adjust vertical pitch. May break some shouts. |
| `DebugLogging` | `0` | Write detailed debug info to the SKSE log. |

---

## How It Works

1. The plugin registers an **animation graph event sink** on the player character.
2. When the `"BeginCastVoice"` animation event fires (the start of any shout animation), the plugin:
   - Checks if the actor is the player.
   - Checks if weapons are sheathed (if `OnlyWhenWeaponsSheathed` is enabled).
   - Reads the camera's yaw direction from the camera node's world transform.
   - Sets the player's facing angle to match the camera direction.
3. The shout then fires normally using the corrected orientation.

The correction happens synchronously on the game thread, before the `"Voice_SpellFire_Event"` that actually launches the shout effect — so the engine uses the updated facing direction.

---

## Known Limitations

- **Yaw only (MVP):** By default, only horizontal aim is corrected. Vertical pitch follows vanilla shout behavior.
- **MatchCameraPitch is experimental:** Enabling pitch correction may cause issues with cone-shaped shouts (Unrelenting Force) or area shouts (Storm Call).
- **Custom scripted shouts:** Some modded shouts that use custom Papyrus scripts or unique projectile systems may not benefit from the rotation correction.
- **Full shout behavior overrides:** If another mod completely overrides shout execution (e.g., replaces the shout function), compatibility is not guaranteed.

---

## Compatibility

### Confirmed Compatible

| Mod | Notes |
|---|---|
| **True Directional Movement** | Camera yaw is read from the actual camera node, not from TDM's free rotation offset. |
| **SmoothCam** | Same — reads the rendered camera direction. |
| **Precision** | No overlap — Precision handles melee; this plugin handles shout aiming. |
| **Improved Camera** | Reads camera world transform, which Improved Camera correctly updates. |
| **Better Third Person Selection** | No conflict — BTPS handles target selection, not shout direction. |

### Design Principles for Compatibility

- Uses **CommonLibSSE-NG** and **Address Library** — no hardcoded offsets.
- Uses the **animation event system** (no function detouring / code patching).
- Does **not** modify shout damage, cooldown, magnitude, duration, or magic effects.
- Does **not** force weapon draw.
- Does **not** affect NPC shouts.
- Does **not** affect melee, archery, spell aiming, or power attacks.

---

## Building from Source

### Prerequisites

- **Visual Studio 2022** with "Desktop development with C++" workload
- **vcpkg** installed with `VCPKG_ROOT` environment variable set
- **CMake 3.21+**

### Build Steps

```powershell
# Clone the repository
git clone https://github.com/your-username/SheathedShoutsAimAtCrosshair.git
cd SheathedShoutsAimAtCrosshair

# Configure (vcpkg will automatically fetch dependencies)
cmake --preset default

# Build (Release)
cmake --build --preset release
```

The compiled DLL and INI will be copied to `dist/SKSE/Plugins/`.

### Auto-Deploy to Mod Manager

Set the `SKYRIM_MODS_FOLDER` environment variable to your MO2 mods directory (e.g., `C:\MO2\mods`). The build will automatically copy the DLL and INI to a `SheathedShoutsAimAtCrosshair` mod folder.

```powershell
$env:SKYRIM_MODS_FOLDER = "C:\MO2\mods"
cmake --build --preset release
```

---

## Logging

The plugin writes to the standard SKSE log directory:  
`Documents/My Games/Skyrim Special Edition/SKSE/SheathedShoutsAimAtCrosshair.log`

**Always logged:**
- Plugin version and runtime detected
- Settings loaded from INI
- Handler registration events (data loaded, new game, save loaded)

**Debug only** (`DebugLogging=1`):
- Every shout aim correction with before/after yaw values
- Skipped corrections (e.g., weapons drawn)

---

## License

MIT License. See [LICENSE](LICENSE) for details.
