cmake_minimum_required(VERSION 3.24)

project(CorsixsModStudio VERSION 0.5.6 LANGUAGES C CXX RC)

if(NOT WIN32)
    message(FATAL_ERROR "Corsix's Mod Studio is a Windows desktop application.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "The legacy solution targets 32-bit Windows. Configure with the Win32 toolchain.")
endif()

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep Lua 5.0 for file-format and script compatibility. Lua 5.1 is loaded at runtime.
set(LUA50_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lua-5.0.2")
file(GLOB LUA50_CORE CONFIGURE_DEPENDS "${LUA50_ROOT}/src/*.c")
list(FILTER LUA50_CORE EXCLUDE REGEX "/ltests\\.c$")
file(GLOB LUA50_STDLIB CONFIGURE_DEPENDS "${LUA50_ROOT}/src/lib/*.c")

add_library(lua50_core STATIC ${LUA50_CORE})
target_include_directories(lua50_core PUBLIC "${LUA50_ROOT}/include" PRIVATE "${LUA50_ROOT}/src")
set_target_properties(lua50_core PROPERTIES OUTPUT_NAME "lua-5.0.2")
add_library(lua50_std STATIC ${LUA50_STDLIB})
target_include_directories(lua50_std PUBLIC "${LUA50_ROOT}/include" PRIVATE "${LUA50_ROOT}/src")
target_link_libraries(lua50_std PUBLIC lua50_core)
set_target_properties(lua50_std PROPERTIES OUTPUT_NAME "lua-5.0.2-std")

set(LUA51_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lua-5.1.2")
file(GLOB LUA51_SOURCES CONFIGURE_DEPENDS "${LUA51_ROOT}/src/*.c")
list(FILTER LUA51_SOURCES EXCLUDE REGEX "/(lua|luac|print)\\.c$")
add_library(lua51_runtime SHARED ${LUA51_SOURCES})
target_include_directories(lua51_runtime PUBLIC "${LUA51_ROOT}/src" "${LUA51_ROOT}/include")
# LUA_CORE and LUA_LIB are defined by the Lua sources themselves; defining them
# here as well only produces C4005 macro-redefinition warnings.
target_compile_definitions(lua51_runtime PRIVATE LUA_BUILD_AS_DLL)
set_target_properties(lua51_runtime PROPERTIES OUTPUT_NAME "lua512" DEBUG_POSTFIX "d")

# Build the optional DXT codec as a runtime DLL with Rainman's legacy byte-RGBA ABI.
set(SQUISH_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsquish")
set(SQUISH_SOURCES
    "${SQUISH_ROOT}/alpha.cpp"
    "${SQUISH_ROOT}/clusterfit.cpp"
    "${SQUISH_ROOT}/colourblock.cpp"
    "${SQUISH_ROOT}/colourfit.cpp"
    "${SQUISH_ROOT}/colourset.cpp"
    "${SQUISH_ROOT}/maths.cpp"
    "${SQUISH_ROOT}/rangefit.cpp"
    "${SQUISH_ROOT}/singlecolourfit.cpp"
    "${SQUISH_ROOT}/squish.cpp")
add_library(squish_runtime SHARED ${SQUISH_SOURCES}
    "${SQUISH_ROOT}/squish_compat.cpp"
    "${SQUISH_ROOT}/squish.def")
target_include_directories(squish_runtime PRIVATE "${SQUISH_ROOT}")
set_target_properties(squish_runtime PROPERTIES PREFIX "" OUTPUT_NAME "squish")
set(CDMS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/CDMS")
set(RAINMAN_ROOT "${CDMS_ROOT}/Rainman")
file(GLOB ZLIB_SOURCES CONFIGURE_DEPENDS "${CDMS_ROOT}/zlib/*.c")
add_library(modstudio_zlib STATIC ${ZLIB_SOURCES})
target_include_directories(modstudio_zlib PUBLIC "${CDMS_ROOT}/zlib")

file(GLOB RAINMAN_SOURCES CONFIGURE_DEPENDS "${RAINMAN_ROOT}/*.cpp" "${RAINMAN_ROOT}/*.c")
list(FILTER RAINMAN_SOURCES EXCLUDE REGEX "/CLuaScript_Interface\\.cpp$")
# The version resource for Rainman.dll; the glob above only collects C/C++.
list(APPEND RAINMAN_SOURCES "${RAINMAN_ROOT}/version.rc")
add_library(Rainman SHARED ${RAINMAN_SOURCES})
target_compile_definitions(Rainman PRIVATE RAINMAN_EXPORTS LUA_BUILD_AS_DLL)
target_include_directories(Rainman
    PUBLIC "${RAINMAN_ROOT}/include"
    PRIVATE "${RAINMAN_ROOT}" "${CDMS_ROOT}" "${LUA51_ROOT}/src")
target_link_libraries(Rainman PRIVATE lua50_core lua50_std lua51_runtime modstudio_zlib Shlwapi)

set(WXWIDGETS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/out/deps/wxWidgets-2.8.12" CACHE PATH
    "Path to the wxWidgets 2.8.12 source tree built with MSVC")
set(WXPROPGRID_ROOT "${WXWIDGETS_ROOT}/contrib" CACHE PATH
    "Path to wxWidgets contrib containing patched wxPropertyGrid 1.2.6")
set(WX_LIB_DIR "${WXWIDGETS_ROOT}/lib/vc_lib")

if(NOT EXISTS "${WXWIDGETS_ROOT}/include/wx/wx.h")
    message(FATAL_ERROR
        "wxWidgets 2.8.12 was not found at:\n  ${WXWIDGETS_ROOT}\n"
        "Run bootstrap-wxwidgets.ps1, or set WXWIDGETS_ROOT to a built Win32 Unicode tree.")
endif()

set(_wx_release_libs
    wxmsw28u_core.lib wxbase28u.lib wxbase28u_net.lib wxbase28u_xml.lib
    wxmsw28u_adv.lib wxmsw28u_aui.lib wxmsw28u_html.lib wxmsw28u_xrc.lib
    wxmsw28u_stc.lib wxmsw28u_propgrid.lib
    wxjpeg.lib wxpng.lib wxtiff.lib wxzlib.lib wxexpat.lib wxregexu.lib)
set(_wx_debug_libs
    wxmsw28ud_core.lib wxbase28ud.lib wxbase28ud_net.lib wxbase28ud_xml.lib
    wxmsw28ud_adv.lib wxmsw28ud_aui.lib wxmsw28ud_html.lib wxmsw28ud_xrc.lib
    wxmsw28ud_stc.lib wxmsw28ud_propgrid.lib
    wxjpegd.lib wxpngd.lib wxtiffd.lib wxzlibd.lib wxexpatd.lib wxregexud.lib)

# A Visual Studio solution carries every configuration at once, so the Debug
# flavour of wxWidgets (mswud) is only needed when the solution offers Debug.
# Collecting the missing files keeps the diagnostics accurate and lets a
# Release-only checkout configure without waiting for a Debug wx build.
function(_cdms_collect_missing_wx_libs out_var libs setup_header)
    set(_missing "")
    foreach(_wx_lib IN LISTS libs)
        if(NOT EXISTS "${WX_LIB_DIR}/${_wx_lib}")
            list(APPEND _missing "${WX_LIB_DIR}/${_wx_lib}")
        endif()
    endforeach()
    if(NOT EXISTS "${WX_LIB_DIR}/${setup_header}")
        list(APPEND _missing "${WX_LIB_DIR}/${setup_header}")
    endif()
    set(${out_var} "${_missing}" PARENT_SCOPE)
endfunction()

_cdms_collect_missing_wx_libs(_wx_missing_release "${_wx_release_libs}" "mswu/wx/setup.h")
if(_wx_missing_release)
    string(REPLACE ";" "\n  " _wx_missing_release_text "${_wx_missing_release}")
    message(FATAL_ERROR
        "The Release wxWidgets libraries are incomplete:\n  ${_wx_missing_release_text}\n"
        "Run:\n  .\\bootstrap-wxwidgets.ps1 -Configuration Release")
endif()

option(CDMS_REQUIRE_DEBUG_WX
    "Treat missing Debug wxWidgets libraries as a configuration error" OFF)

_cdms_collect_missing_wx_libs(_wx_missing_debug "${_wx_debug_libs}" "mswud/wx/setup.h")
get_property(_cdms_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_wx_missing_debug)
    if(CDMS_REQUIRE_DEBUG_WX OR (NOT _cdms_multi_config AND CMAKE_BUILD_TYPE STREQUAL "Debug"))
        string(REPLACE ";" "\n  " _wx_missing_debug_text "${_wx_missing_debug}")
        message(FATAL_ERROR
            "The Debug wxWidgets libraries are incomplete:\n  ${_wx_missing_debug_text}\n"
            "Run:\n  .\\bootstrap-wxwidgets.ps1 -Configuration Debug")
    endif()
    if(_cdms_multi_config)
        if("Debug" IN_LIST CMAKE_CONFIGURATION_TYPES)
            list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES "Debug")
            set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
                "Configurations offered by the generated solution" FORCE)
            # Remembered so that Debug is added back once the Debug wxWidgets
            # libraries exist, instead of staying hidden for good.
            set(CDMS_WX_DEBUG_CONFIG_REMOVED TRUE CACHE INTERNAL
                "Debug was removed from the solution because the Debug wxWidgets libraries were missing")
            message(STATUS
                "Debug wxWidgets libraries are not built yet, so the solution offers "
                "'${CMAKE_CONFIGURATION_TYPES}' only. Run\n"
                "  .\\bootstrap-wxwidgets.ps1 -Configuration Debug\n"
                "and re-run CMake to add the Debug configuration.")
        endif()
    endif()
elseif(_cdms_multi_config AND CDMS_WX_DEBUG_CONFIG_REMOVED)
    set(CMAKE_CONFIGURATION_TYPES "Debug;${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Configurations offered by the generated solution" FORCE)
    unset(CDMS_WX_DEBUG_CONFIG_REMOVED CACHE)
    message(STATUS "Debug wxWidgets libraries are available; the Debug configuration is back.")
endif()

set(WX_INCLUDE_DIRS
    "${WXWIDGETS_ROOT}/include"
    "${WXWIDGETS_ROOT}/contrib/include"
    "${WXPROPGRID_ROOT}/include")
set(_wx_setup_include "$<IF:$<CONFIG:Debug>,${WX_LIB_DIR}/mswud,${WX_LIB_DIR}/mswu>")
set(_wx_lib_d "$<$<CONFIG:Debug>:d>")
set(_wx_link_libs
    "${WX_LIB_DIR}/wxmsw28u${_wx_lib_d}_core.lib"
    "${WX_LIB_DIR}/wxbase28u${_wx_lib_d}.lib"
    "${WX_LIB_DIR}/wxbase28u${_wx_lib_d}_net.lib"
    "${WX_LIB_DIR}/wxbase28u${_wx_lib_d}_xml.lib"
    "${WX_LIB_DIR}/wxmsw28u${_wx_lib_d}_adv.lib"
    "${WX_LIB_DIR}/wxmsw28u${_wx_lib_d}_aui.lib"
    "${WX_LIB_DIR}/wxmsw28u${_wx_lib_d}_html.lib"
    "${WX_LIB_DIR}/wxmsw28u${_wx_lib_d}_xrc.lib"
    "${WX_LIB_DIR}/wxmsw28u${_wx_lib_d}_stc.lib"
    "${WX_LIB_DIR}/wxmsw28u${_wx_lib_d}_propgrid.lib"
    "${WX_LIB_DIR}/wxjpeg${_wx_lib_d}.lib"
    "${WX_LIB_DIR}/wxpng${_wx_lib_d}.lib"
    "${WX_LIB_DIR}/wxtiff${_wx_lib_d}.lib"
    "${WX_LIB_DIR}/wxzlib${_wx_lib_d}.lib"
    "${WX_LIB_DIR}/wxexpat${_wx_lib_d}.lib"
    "${WX_LIB_DIR}/wxregexu${_wx_lib_d}.lib")

file(GLOB CDMS_SOURCES CONFIGURE_DEPENDS "${CDMS_ROOT}/CDMS/*.cpp")
add_executable(CorsixsModStudio WIN32
    ${CDMS_SOURCES}
    "${CDMS_ROOT}/CDMS/art/stuff.rc"
    "${CDMS_ROOT}/CDMS/art/CorsixsModStudio.manifest")
add_dependencies(CorsixsModStudio lua51_runtime squish_runtime)
target_include_directories(CorsixsModStudio PRIVATE
    "${_wx_setup_include}"
    ${WX_INCLUDE_DIRS}
    "${RAINMAN_ROOT}/include"
    "${CDMS_ROOT}"
    "${CDMS_ROOT}/CDMS"
    "${CDMS_ROOT}/CDMS/art")
target_compile_definitions(CorsixsModStudio PRIVATE
    UNICODE _UNICODE WIN32_LEAN_AND_MEAN __WXMSW__
    $<$<CONFIG:Debug>:__WXDEBUG__>)
target_link_libraries(CorsixsModStudio PRIVATE
    Rainman lua50_core lua50_std modstudio_zlib
    ${_wx_link_libs}
    comctl32 oleacc rpcrt4 shell32 shlwapi winmm ws2_32)
set_target_properties(CorsixsModStudio PROPERTIES
    OUTPUT_NAME "CorsixsModStudio"
    VS_DPI_AWARE "ON"
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CorsixsModStudio>")

# The application expects its data under ./Mod_Studio_Files and Lua 5.1 at
# legacy names selected by build configuration.
add_custom_command(TARGET CorsixsModStudio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CDMS_ROOT}/CDMS/files" "$<TARGET_FILE_DIR:CorsixsModStudio>/Mod_Studio_Files"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:lua51_runtime>" "$<TARGET_FILE_DIR:CorsixsModStudio>/Mod_Studio_Files/lua512.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:squish_runtime>" "$<TARGET_FILE_DIR:CorsixsModStudio>/Mod_Studio_Files/squish.dll"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:CorsixsModStudio>/Mod_Studio_Files/debug"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:lua51_runtime>"
        "$<TARGET_FILE_DIR:CorsixsModStudio>/Mod_Studio_Files/$<IF:$<CONFIG:Debug>,debug/lua512d.dll,lua512p.dll>"
    VERBATIM)

install(TARGETS CorsixsModStudio Rainman RUNTIME DESTINATION . LIBRARY DESTINATION .)
install(TARGETS squish_runtime RUNTIME DESTINATION Mod_Studio_Files LIBRARY DESTINATION Mod_Studio_Files)
install(DIRECTORY "${CDMS_ROOT}/CDMS/files/" DESTINATION Mod_Studio_Files)
# Rainman.dll has lua512.dll in its import table, so the loader needs a copy
# beside the executable. Mod_Studio_Files keeps the renamed copies the
# application itself loads for Lua 5.1 support.
install(FILES "$<TARGET_FILE:lua51_runtime>" DESTINATION .)
install(FILES "$<TARGET_FILE:lua51_runtime>" DESTINATION Mod_Studio_Files RENAME lua512.dll)
install(FILES "$<TARGET_FILE:lua51_runtime>"
    DESTINATION Mod_Studio_Files/debug RENAME lua512d.dll CONFIGURATIONS Debug)
install(FILES "$<TARGET_FILE:lua51_runtime>"
    DESTINATION Mod_Studio_Files RENAME lua512p.dll CONFIGURATIONS Release RelWithDebInfo MinSizeRel)

# Release optimization tuned for general 32-bit Windows PCs. CMake's MSVC
# Release configuration supplies /O2; SSE2 is the x86 compatibility baseline
# used by mainstream Windows systems. Avoid machine-specific AVX tuning.
if(MSVC)
    foreach(target IN ITEMS
        lua50_core lua50_std lua51_runtime modstudio_zlib Rainman
        squish_runtime CorsixsModStudio)
        target_compile_options(${target} PRIVATE
            /MP
            $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/O2>
            $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/arch:SSE2>)
    endforeach()
endif()
