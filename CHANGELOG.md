# Changelog

## 0.5.6

A maintenance release: the 0.5.5 application, built with a current toolchain,
with the features that 0.5.5 shipped but never connected now working. File
formats, UI layout, strings, artwork and the bundled documentation are
unchanged; `PORTING.md` records every source edit against the 0.5.5 drop.

### Build

- Builds with CMake and a 32-bit MSVC toolchain from Visual Studio 2026 instead
  of the Visual Studio 2005 solution, which is kept for reference only.
- `bootstrap-wxwidgets.ps1` downloads and builds wxWidgets 2.8.12 (checksum
  pinned) when no tree is present, and can use a local archive with `-Source`.
- wxPropertyGrid 1.2.6 is bundled in `third_party/wxpropgrid`, because
  wxWidgets 2.8.x never shipped it and its download page is gone.
- A Release-only checkout configures without the Debug wxWidgets libraries;
  Debug reappears automatically once they are built.
- `Rainman.dll` carries its 0.5.5 version resource again, so its file
  properties show version and licence information.
- The build is warning-free in Release and Debug.

### Fixed

- **File → New mod** did nothing: the menu used `wxID_NEW` with no handler.
- **Help → About** did nothing: the menu used `wxID_ABOUT` with no handler.
  It now shows the version, website and copyright.
- **Mod → Properties** did nothing; it now selects the module tab, and recreates
  that tab if it was closed with its X.
- The module properties **"?" version-help button** did nothing; it now shows
  the 0.5.5 help text for the version field.
- The unhandled-exception path in `CDMSApplication::OnRun` returned no status
  code.
- Missing Squish DXT codec produced a bare load error; the message now says
  which features stop working. The library is loaded quietly.
- **Mod properties are now saved.** Editing the description, internal name,
  texture fields or version only changed the in-memory module; there was no save
  path at all, so the changes disappeared when the mod was closed. The properties
  tab now has a **Save** button, tracks modifications and asks before closing.
  Only those five values are rewritten - every other line of the `.module` file,
  including comments and Company of Heroes `[data:...]` sections, is preserved
  byte for byte. Mods opened from an SGA archive have no `.module` file to
  update and say so instead of failing silently.
- Removed the leftover **"Developer's Red Button"** tool from the Mod menu. It
  was a debug stub that tried to write to the hardcoded path
  `I:\j\k\l\m\n\o\p\q.test`, so it only ever produced an error.
- **Help menu cleaned up.** Six entries pointed at community sites that have
  been gone for years (RDN wiki x2, both Relicnews forums, the Lua reference and
  Kresjah's wiki). They are replaced by a single **Project page on GitHub** link
  next to the local help pages.
- **Relic's Tools entries are disabled when the tool is not installed** (the
  executables are searched for in the mod folder, `ModTools` and the configured
  game folders). Launching a missing one now explains that instead of doing
  nothing.
- **Editor shortcuts**: `Ctrl+S` saves the active editor, `Ctrl+W` closes the
  active tab (with the usual save prompt) and `Ctrl+Tab` cycles tabs.

### Robustness

- **Closing a mod tore down the wrong way round**: the module was deleted before
  the tabs and file tree that borrow pointers into its file map. The views are
  now destroyed first, mirroring the fix in the Definitive Edition fork.
- **`Util_fgetline` could read before its buffer** on an empty file or a file
  that does not end in a newline (`strlen(...) - 1` on an empty read). The
  buffer now starts as an empty string and the accumulated line length is
  checked.
- **Mass Extract** passed conflicting sizer flags (`wxALIGN_LEFT |
  wxALIGN_CENTER_VERTICAL`) to its list box, a known assertion/crash source.
- **RGD and Lua child sorting no longer rewrites the names it compares.** The
  comparator used to zero the underscore in place and restore it, which is
  unsafe for shared or read-only strings; it now compares prefixes without
  touching the data.

### Data safety

- **File → Close mod** deleted every editor tab without asking. It now sends a
  close event to each tab first, exactly like closing the application, so
  modified RGD, UCS, SCAR and material files prompt to save.
- **Clicking a tab's X** removed the page without asking as well (wxAuiNotebook
  does not send a close event by itself). The page is now asked first, so an
  unsaved editor or the mod-properties tab prompts instead of discarding.
- The **RGM material editor** did not track changes at all: edits were lost
  silently when the tab or the mod was closed. It now tracks modifications and
  asks before closing.
- When a save fails (read-only mod folder, required-mod file, disk error), the
  RGD, UCS, SCAR and material editors keep the tab open instead of closing and
  discarding the edit.

### Startup and paths

- An **incomplete installation** (missing `Mod_Studio_Files`, `lua.dat`,
  `bfx.dat`, `scardoc.dat` or the RGD dictionary) now shows one dialog listing
  the missing files and exits. Previously it produced a chain of library-error
  dialogs and left an unusable window behind.
- Paths are converted through the **system ANSI code page** instead of being
  truncated to Latin-1. On localized Windows (Cyrillic, Greek, CJK, ...) mods
  and folders with such names now open and save correctly; previously every
  character above Latin-1 became `?` and the path did not exist.
- Paths the code page cannot represent are refused with an explanation at every
  point where the user picks one (open mod, open SGA, New mod destination, mass
  extract, SGA packer input/output, AE setup output, RGD macro save/load and the
  texture "Save Copy" destination) instead of quietly operating on a different
  path.

### Changed

- The process working directory is set to the executable's folder on startup,
  so shortcuts, the Start menu and Explorer launches find `Mod_Studio_Files`.
- The shipped `config.txt` no longer contains the original author's machine
  paths and per-mod locale history; it starts from first-run defaults.

### Packaging

- `package-win32.ps1` produces a self-contained folder and zip under
  `out/dist`, including `lua512.dll` beside the executable as `Rainman.dll`
  requires.

## 0.5.5

The original release by Corsix and the Rainman development team. Its untouched
sources are kept in `../reference/CDMSSrc_055`.
