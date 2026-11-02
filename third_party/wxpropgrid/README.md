# wxPropertyGrid 1.2.6 (bundled)

Corsix's Mod Studio builds its property grid from wxPropertyGrid 1.2.6, the
standalone library by Jaakko Salli that later became part of wxWidgets 2.9.
wxPropertyGrid was never shipped inside the wxWidgets 2.8.x release archives,
and the download page that the original 0.5.5 build instructions referred to is
gone, so the sources this project compiles are kept here.

## Layout

| Path | Installed to |
| --- | --- |
| `include/wx/propgrid/` | `<wx root>/contrib/include/wx/propgrid/` |
| `src/propgrid/` | `<wx root>/contrib/src/propgrid/` |
| `build/propgrid/makefile.vc` | `<wx root>/contrib/build/propgrid/` |

`bootstrap-wxwidgets.ps1` copies this tree into the wxWidgets 2.8.12 sources
after unpacking them, then applies the three edited files kept in
`../wxpropgrid-modifications`. The sources here are unmodified upstream 1.2.6,
so the patch step is what produces the property description/help-caption API
the application needs; keeping the patches separate means the bundled sources
can be compared with, or replaced by, a newer upstream snapshot.

The contents come from the original `wxpropgrid-1.2.6-src.tar.gz` distribution,
files unmodified. The three files the patches replace are:

| File | SHA-256 of the bundled (unpatched) copy |
| --- | --- |
| `src/propgrid/propgrid.cpp` | `4DE8F65EF503AEA1CCCE0A35B602BB055D6A506BDEFE88A2D736B73A1ED3ABFC` |
| `src/propgrid/manager.cpp` | `8EF4F82542E37990CFAEF03B5A6D47181D18942AA73C8DBF75E9072D7FE4BDD5` |
| `include/wx/propgrid/propgrid.h` | `31F155CE4F658331F8A006756C5592F910A5BBBC6B5ACE98E3ADA4CC3C0B0B68` |

Only what the library needs to compile is kept: the sources, the public
headers, and the Visual C++ makefile. The samples, documentation, other-platform
makefiles and the Visual Studio 2003-2005 project files from the original
distribution are not included.

## Licence

wxPropertyGrid is distributed under the wxWindows Library Licence, which is the
same licence wxWidgets uses. The notice is repeated at the top of every source
file in this directory.
