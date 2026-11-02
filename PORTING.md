# Porting notes: 0.5.5 sources to a current MSVC toolchain

Corsix's Mod Studio 0.5.5 was written for Visual Studio 2003/2005. This tree
keeps those sources, but builds them with a current MSVC (32-bit) through CMake,
and reports itself as version 0.5.6 (`CHANGELOG.md` lists what changed). The
goal is that the program behaves like 0.5.5, so changes to the original sources
are kept to the smallest set that a modern compiler and SDK require, plus a
handful of features that 0.5.5 shipped but never wired up.

Everything not listed here is byte-identical to the 0.5.5 sources in
`../reference/CDMSSrc_055`: window layout, string tables, dialogs, artwork, the
Rainman file formats, the Lua 5.0.2 and 5.1.2 sources, zlib and the bundled Lua
scripts.

## Source layout

| 0.5.5 (`../reference/CDMSSrc_055`) | This repository |
| --- | --- |
| `CDMS_src/` | `CDMS/CDMS/` |
| `CDMS_art/` | `CDMS/CDMS/art/` |
| `CDMS_files/` | `CDMS/CDMS/files/` |
| `Rainman_src/` | `CDMS/Rainman/` |
| `Rainman_src/zLib/` | `CDMS/zlib/` (built as `modstudio_zlib`) |
| `wxPropgrid-modifications/` | `third_party/wxpropgrid-modifications/` |

## Edits required by a modern compiler, SDK and wxWidgets build

| File | Change | Reason |
| --- | --- | --- |
| `art/stuff.rc` | `#include "afxres.h"` → `<winres.h>` | `afxres.h` belongs to MFC, which this project does not use. |
| `art/CorsixsModStudio.manifest` | Added | Declares Common Controls 6, `asInvoker`, supported OS versions, and system DPI awareness so the window is not bitmap-scaled on high-DPI displays. |
| `Rainman/Lua51.h` | Absolute `e:\CPP\lua-5.1.2\src\luaconf.h` → `"luaconf.h"` | The hard-coded path only existed on the author's machine (the repository copy of this file had a different absolute path again); the include is now found through the Lua 5.1.2 include directories. |
| `Rainman/include/Rainman.h` | Dropped `#include "../CIniConfigFile.h"` | That header is not part of the 0.5.5 sources. |
| `CDMS/Tools.cpp` | Dropped `#include <Rainman_RGDDump.h>` | Same: the header was never shipped, and its only consumer (`RgdDump::CRgd`) sits inside a commented-out block. |
| `CDMS/frmFiles.cpp` | zlib include `../../Rainman/src/zLib/zlib.h` → `../zlib/zlib.h` | Follows the moved zlib tree. |
| `Rainman/CInheritTable.cpp`, `CLuaFile2.cpp`, `CRgdFile.cpp`, `CRgtFile.cpp`, `CSgaCreator.cpp`, `CSgaFile.cpp` | `#include "zLib/zlib.h"` → `#include "..\zLib/zlib.h"` | Same moved zlib tree; the include is relative to the Rainman sources. |
| `Rainman/version.rc` | `#include "afxres.h"` → `<winres.h>`; the file is now compiled into `Rainman.dll` | Same MFC-free reason as `stuff.rc`. 0.5.5 used this resource to stamp `Rainman.dll` with its version and LGPL notice; the port had left it out of the build, so the DLL carried no version information. |
| `CDMS/Construct.cpp` | `../..\Rainman\src\md5.h` → `..\Rainman\md5.h`; added `<shellapi.h>` and `<wx/aboutdlg.h>` | Path follows the new layout; current SDKs and wxWidgets no longer bring these declarations in indirectly. |
| `CDMS/frmRgdEditor.cpp` | `GetNextSibling(...)` calls qualified as `wxPropertyContainerMethods::GetNextSibling(...)` | `wxPropertyGrid` inherits `GetNextSibling` from both `wxPropertyContainerMethods` (one `wxPGId` argument) and, through `wxScrolledWindow`, from `wxWindowBase` (one `wxWindow*` argument). Current MSVC reports C2385 (ambiguous access); naming the base selects the same function the old compiler picked. |
| `CDMS/frmFiles_Actions.h` | `char* sTmp = strrchr(...)` → `const char* sTmp` | `strrchr` returns `const char*` in current CRT headers. |
| `Rainman/CLuaFile.cpp`, `Rainman/CLuaFromRgd.cpp` | Inline `__asm nop` / `__asm { nop; }` barriers → `std::atomic_signal_fence(std::memory_order_seq_cst)` | The originals are MSVC-x86-only and stop the x64-capable toolchain from compiling them. The fence keeps the intent (an optimizer barrier at that point) in portable form. |

## Features that 0.5.5 had but never connected

These menu items and buttons existed in 0.5.5 with no event handler, so they did
nothing. The handlers below make the shipped UI work as documented; they do not
add new windows.

| UI element | 0.5.5 behaviour | Now |
| --- | --- | --- |
| File → New mod | Menu used `wxID_NEW` with no handler. | New `IDM_NewMod` id calls the existing `DoNewMod()`. |
| Help → About | Menu used `wxID_ABOUT` with no handler. | New `IDM_About` id shows a standard About box with the version, website and copyright already present in the string table. |
| Mod → Properties | `wxID_PROPERTIES` had no handler. | Selects the module tab, which is where the mod properties live. |
| Module properties "?" button | Button `IDC_VersionHelp` had no handler. | Shows the existing `mod_version_help` text. |

Two smaller fixes belong here as well:

- The Squish DXT loader now uses `wxDL_QUIET` and explains what stops working
  when the library is missing instead of showing a bare load error.
- `CDMSApplication::OnRun` returns `-1` after reporting an unhandled exception;
  the original fell through without a return value.

## Deliberate behaviour changes

- **Working directory.** `CDMSApplication::OnInit` sets the process working
  directory to the folder that contains the executable before reading
  `Mod_Studio_Files/config.txt`. 0.5.5 assumed it was started from its own
  folder, which is false for shortcuts, the Start menu and Explorer, and the
  shipped config file then silently could not be found.
- **`CDMS/CDMS/files/config.txt`.** Reset to first-run defaults. The 0.5.5 copy
  contained the author's own `E:\Dawn of War - Dark Crusade\` and
  `F:\Dawn of War\` paths plus per-mod locale history keyed by mod hash, none of
  which is meaningful in a fresh install.

## Robustness fixes (0.5.6)

These go beyond compiler compatibility: they stop the program from losing work
or walking into a dead end. Each one either reuses an existing 0.5.5 code path
that was never called, or adds a guard around an existing one.

| File | Change | Why |
| --- | --- | --- |
| `CDMS/Construct.cpp`, `Construct.h` | New `ConfirmCloseTabs()`; `OnCloseMod` runs it before `SetModule(0)`; new `OnClosePage` handler for `EVT_AUINOTEBOOK_PAGE_CLOSE` | Closing a mod deleted every tab with `DeletePage()`, discarding unsaved RGD/UCS/SCAR/material edits. The close-event loop already existed for application exit; it is now shared. Clicking a tab's X bypassed every prompt because wxAuiNotebook closes pages itself without sending a close event; the new handler asks the page and vetoes the notebook event instead. |
| `CDMS/frmRgdEditor.cpp`, `frmUCSEditor.cpp`, `frmScarEditor.cpp` | After a "save on close" attempt, veto the close when the modified flag is still set | A failed save closed the tab anyway, losing the edit. |
| `CDMS/frmRgmMaterialEditor.cpp`, `.h` | Added `m_bNeedsSave`, `EVT_CLOSE` and a save prompt | The material editor tracked no changes at all, so edits vanished on close. |
| `CDMS/Application.cpp` | Startup check for the data files (and `OnExit` guards a missing config); refuses to start with one dialog | A half-copied installation produced a chain of library-error dialogs and an unusable frame. |
| `CDMS/strconv.cpp`, `.h` | `UnicodeToAscii` converts through `CP_ACP`; new `IsAnsiRepresentable()` | The old one-byte truncation turned everything above Latin-1 into `?`, so mods under localized paths could not be opened at all. |
| `CDMS/Utility.cpp`, `.h` | New `CheckAnsiPath()` | Reports unrepresentable paths with one message instead of silently opening a different path. |
| `CDMS/Construct.cpp`, `frmNewMod.cpp`, `frmMassExtract.cpp`, `frmSgaMake.cpp`, `Tool_AESetup.cpp`, `frmRgdMacro.cpp`, `frmImage.cpp` | Call `CheckAnsiPath()` on every path the user picks | Open mod, open SGA, New mod destination, mass extract, SGA packer input/output, AE setup output, RGD macro save/load and texture "Save Copy". |
| `CDMS/strings.h` | New strings (`app_datamissing*`, `path_notansi*`, `modsave_*`) | User-facing text for the messages above, kept in the existing string table. |
| `CDMS/Rainman/ModuleFileWriter.cpp`, `.h` | New helper that rewrites the five user-editable values in a `.module` file and leaves every other line untouched; writes through a temp file and swaps it in | There was no module writer reachable from the GUI, so the properties tab could not persist anything. Regenerating the whole file (as `CDoWModule::Save` does) would drop comments, unknown directives and CoH sections; a targeted rewrite keeps mods intact. |
| `CDMS/Rainman/CModuleFile.cpp`, `.h` | New `SaveModuleFile()` calling the helper above | `CModuleFile` (the class the GUI actually loads) had no save path; `CDoWModule::Save` belongs to a different, older class. |
| `CDMS/frmModule.cpp`, `.h` | Save button, `m_bNeedsSave`, `EVT_CLOSE` prompt, version-field handlers, dirty marking in every edited field | The properties tab edited only the in-memory module and the version text boxes were not read at all, so changes were lost on close. |
| `CDMS/Tools.cpp`, `Tools.h`, `Construct.cpp`, `strings.h` | Removed the "Developer's Red Button" debug tool and its string | Its `DoAction` wrote to the hardcoded path `I:\j\k\l\m\n\o\p\q.test` and could never succeed. |
| `CDMS/Construct.cpp` (`SetModule`) | Destroy the tabs and the file tree before deleting the module | The file tree's item data borrows `sMod`/`sSource` pointers from the module's file map, so the old order left dangling pointers during Close Mod. |
| `CDMS/Rainman/Internal_Util.cpp` (`Util_fgetline`) | Initialise the buffer to an empty string; trim the newline using the accumulated length | On an empty file, or a file ending without a newline, `strlen(sReadTo) - 1` indexed before the buffer. |
| `CDMS/frmMassExtract.cpp` | Removed `wxALIGN_CENTER_VERTICAL` from the check-list sizer flags | Conflicting alignment flags on a list box are a known assertion/crash source. |
| `CDMS/Rainman/CRgdFile.cpp`, `CLuaFile.cpp` (`_SortCMetaTableChildren`) | Compare prefixes with `strnicmp` instead of writing `'\0'` over the underscore and restoring it | Mutating the strings inside a sort comparator is unsafe for shared or read-only data. |
| `CDMS/Construct.cpp`, `Construct.h`, `strings.h` | Help menu: six dead 2008 links replaced by one "Project page on GitHub"; `IDM_ProjectPage` | The removed sites no longer resolve. |
| `CDMS/Construct.cpp`, `Construct.h`, `strings.h` | `FindRelicTool()` / `UpdateRelicToolsMenu()`: search the mod folder, `ModTools` and the configured game folders; disable missing entries; explain a missing tool on launch | Every Relic tool menu item was enabled even when the executable did not exist, so clicking it did nothing. |
| `CDMS/Construct.cpp`, `Construct.h` | Accelerator table: `Ctrl+S` save active editor, `Ctrl+W` close active tab, `Ctrl+Tab` next tab; `OnSaveActive` dispatches by editor type | The editors had no keyboard shortcuts; dispatching by type (not by synthesised button id) avoids id collisions between pages. |

## Version identity

The build identifies itself as 0.5.6 rather than 0.5.5. That changes the version
data only — no behaviour:

| File | Change |
| --- | --- |
| `CMakeLists.txt` | `project(... VERSION 0.5.6)` drives the package name |
| `CDMS/CDMS/strings.h` | `app_version` is `"0.5.6"`, so the title bar, About box and version fields read 0.5.6 |
| `CDMS/CDMS/art/stuff.rc` | `FILEVERSION`/`PRODUCTVERSION` and the `FileVersion`/`ProductVersion` strings are `0, 5, 6, 0` |
| `CDMS/CDMS/art/CorsixsModStudio.manifest` | Assembly version is `0.5.6.0` |
| `CDMS/Rainman/version.rc` | `SpecialBuild` reads "For Mod Studio 0.5.6"; Rainman keeps its own library version, 1.5.5.0 |

The 0.5.5 help pages under `CDMS/CDMS/files/docs` are the released 2008
documentation and keep their historical text. `CHANGELOG.md` describes what
0.5.6 changes.

## Checking fidelity

Compare the ported sources with the 0.5.5 reference tree at any time:

```powershell
$orig = 'C:\Users\IgorTheLight\Documents\Codex\reference\CDMSSrc_055'
git diff --no-index --ignore-cr-at-eol --stat -- "$orig\CDMS_src" .\CDMS\CDMS
git diff --no-index --ignore-cr-at-eol --stat -- "$orig\Rainman_src" .\CDMS\Rainman
```

Expected result: text changes only in the files listed above, plus the new
`art/` and `files/` content and the moved zlib tree.
