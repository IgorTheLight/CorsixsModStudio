# Corsix's Mod Studio 0.5.7

This repository contains the Windows desktop source for Corsix's Mod Studio and its Rainman library. The original Visual Studio 2005 solution remains for historical reference. The maintained build path uses CMake and the Win32 (x86) MSVC toolchain. C++ modernization is intentionally out of scope for this port: the goal is a buildable, runnable 0.5.7 — the 0.5.5 feature set plus build fixes and the shipped-but-unconnected UI fixes — not a redesign. `PORTING.md` lists every source edit against the original 0.5.5 release and why it exists, and `CHANGELOG.md` summarises what 0.5.7 changes.

## Repository layout

| Path | Contents |
| --- | --- |
| `CDMS/CDMS/` | Application sources, resources and the `Mod_Studio_Files` data the program ships |
| `CDMS/Rainman/` | Rainman shared library (mod, SGA, RGD, RGT, UCS and Lua file formats) |
| `CDMS/zlib/` | zlib as compiled into the application and Rainman |
| `lua-5.0.2/`, `lua-5.1.2/` | Lua runtimes; 5.0.2 for file formats, 5.1.2 for the scripting host |
| `third_party/` | Bundled third-party sources (libsquish, wxPropertyGrid 1.2.6 and its patches) |
| `out/deps/` | Built dependencies such as the wxWidgets tree (gitignored) |
| `out/build/` | CMake build tree (gitignored) |
| `out/dist/` | Packaged releases from `package-win32.ps1` (gitignored) |

The untouched 0.5.5 sources used as the fidelity reference live next to this repository in `../reference/CDMSSrc_055`.

## Dependencies

- Visual Studio 2026 with **Desktop development with C++**, an x86 MSVC toolset, and a Windows SDK.
- CMake 3.24 or newer.
- wxWidgets 2.8.12, built as static Unicode Win32 libraries. `bootstrap-wxwidgets.ps1` downloads the official release archive into `out/deps/wxWidgets-2.8.12` when the tree is missing; pass `-Source <zip>` to use a local copy instead. Only the Release libraries are required to build the application; the Debug ones additionally enable wxWidgets' assertion checks and line-level debugging. The helper builds both flavours by default, and CMake drops the Debug configuration from the solution while the Debug libraries are missing.
- wxPropertyGrid 1.2.6, the stand-alone library the property grid needs. It is bundled in `third_party/wxpropgrid` because wxWidgets 2.8.x never included it and its old download page is gone. `bootstrap-wxwidgets.ps1` installs it into the wxWidgets tree.
- wxWidgets' STC contrib library, built from the same wxWidgets tree.

The three files in `third_party/wxpropgrid-modifications` adjust the property description/help-caption API used by the Mod Studio UI. `third_party/wxpropgrid` holds the unmodified upstream 1.2.6 sources, and the bootstrap script overlays them into the wxWidgets tree and then applies the patches.

The old wxTreeCtrl workaround described in the supplied wxWidgets 2.8.0 notes targets wxWidgets 2.8.0. The 2.8.12 tree control has different keyboard-context-menu handling, so that 2.8.0 code change is not applied.

## Build on Windows

```powershell
.\bootstrap-wxwidgets.ps1 -Configuration Release   # downloads wxWidgets, patches and builds it
.\build-win32.ps1
```

`bootstrap-wxwidgets.ps1` builds with `BUILD=release` and `BUILD=debug`, `SHARED=0 UNICODE=1 CPPFLAGS=/DNEED_PBT_H`. The last define skips the old `pbt.h` include because the current Windows SDK declares those constants through `WinUser.h`. Use `-PrepareOnly` to download, unpack and patch without compiling, and `-Force` to rebuild libraries that already exist.

`build-win32.ps1` locates CMake inside the latest Visual Studio installation, so CMake does not need to be on `PATH`. `-BootstrapWx` runs the wxWidgets helper first, and `-Configuration Debug` builds the Debug configuration:

```powershell
.\bootstrap-wxwidgets.ps1 -Configuration Debug
.\build-win32.ps1 -Configuration Debug
```

A Visual Studio solution carries every configuration at once, so the Debug configuration is only generated when the Debug wxWidgets libraries exist. Without them CMake offers Release, RelWithDebInfo and MinSizeRel, prints how to add Debug, and adds it back on the next configure once `bootstrap-wxwidgets.ps1 -Configuration Debug` has run. Pass `-DCDMS_REQUIRE_DEBUG_WX=ON` to treat missing Debug libraries as a configuration error instead.

The executable and required `Mod_Studio_Files` data directory are placed under `out/build/windows-wx28-win32/<Config>`. CMake builds Lua 5.0.2, Lua 5.1.2, Rainman, zlib, and the Squish DXT codec from the sources included here. Lua 5.0.2 is retained for file-format compatibility; Lua 5.1.2 and Squish are copied under the runtime names expected by the application (`Mod_Studio_Files/lua512p.dll`, `Mod_Studio_Files/debug/lua512d.dll`, `Mod_Studio_Files/squish.dll`). Because `Rainman.dll` imports the Lua DLL, a copy also sits beside the executable (`lua512.dll`, or `lua512d.dll` for Debug); the package script fails if either copy is missing. The Squish compatibility layer adapts the byte-RGBA ABI used by Rainman to the vendored libsquish API.

Release and RelWithDebInfo use `/O2` with an SSE2 baseline for broad x86 compatibility. They do not use AVX-specific or host-specific compiler tuning. The application manifest requests system DPI awareness and Common Controls 6 so the window is not bitmap-scaled by Windows. wxWidgets 2.8 itself is not per-monitor DPI aware.

Override `WXWIDGETS_ROOT` if the wxWidgets tree lives elsewhere. The CMake preset uses the Visual Studio 18 2026 Win32 generator; copy `CMakePresets.json` entries into gitignored `CMakeUserPresets.json` to use another VS version.

## Packaging

```powershell
.\package-win32.ps1
```

Builds (unless `-SkipBuild`) and stages the executable, `Rainman.dll`, the runtime libraries and `Mod_Studio_Files` into `out/dist/CorsixsModStudio-0.5.7-win32`, then zips it. `-Configuration Debug` and `-NoArchive` are available; the folder is self-contained and can be copied to another machine.

## Licensing

See `LICENSE.txt`. The original source contains GPL notices for the application and LGPL notices for Rainman. Lua, wxWidgets, wxPropertyGrid, libsquish, zlib, the artwork, and other bundled assets have their own terms. Review the relevant notices before redistributing a combined build.
