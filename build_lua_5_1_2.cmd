        :: @echo off

        setlocal
        set lua_version=5.1.2
        set work_dir=%~dp0
        set work_dir=%work_dir:~0,-1%
        set lua_build_dir=%work_dir%\lua-%lua_version%
        set compiler_bin_dir=C:\TDM-GCC-32\bin
        set path=%compiler_bin_dir%;%path%

        cd /D %lua_build_dir%
        mingw32-make PLAT=mingw

        :: create a clean "binary" installation
        mkdir %lua_build_dir%\bin

        copy %lua_build_dir%\src\*.exe %lua_build_dir%\bin\*.*
        copy %lua_build_dir%\src\*.dll %lua_build_dir%\bin\*.*

        echo.
        echo **** BINARY DISTRIBUTION BUILT ****
        echo.

        %lua_build_dir%\bin\lua.exe -e"print [[Hello!]];print[[Simple Lua test successful!!!]]"

        echo.

        pause