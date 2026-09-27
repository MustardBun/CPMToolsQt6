cmake_minimum_required(VERSION 3.21)

project(CPMToolsQt6
    VERSION 0.1.0
    DESCRIPTION "CPMToolsQt6 - CP/M disk image tools, Qt6 port of CPMToolsGUI"
    LANGUAGES C CXX)

# Release tag used in artefact names. Kept separate from the numeric project
# VERSION because CMake requires VERSION to be numeric only.
set(CPM_VERSION_TAG "v0.1-preview")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Statically link the GCC/C++ runtime into the command line tool so it becomes a
# single self-contained file. The GUI is left dynamic: it ships the Qt DLLs,
# which need the shared MinGW runtime anyway (see CPM_PORTABLE).
option(CPM_STATIC_RUNTIME "Statically link the GCC/C++ runtime into cpmcli" ON)

# ---------------------------------------------------------------------------
# Core engine - portable C fork of cpmtools (device_posix backend).
# These files are shared with the original C++Builder project, unmodified.
# ---------------------------------------------------------------------------
set(CPM_CORE_SOURCES
    cpmfs.c
    cpmcp.c
    cpm_test.c
    mkfs.cpm.c
    device_posix.c)

add_library(cpmcore STATIC ${CPM_CORE_SOURCES})

target_include_directories(cpmcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_definitions(cpmcore PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    # The engine is legacy K&R-style C. Silence warning noise (but not errors)
    # and allow tentative definitions as older compilers did.
    target_compile_options(cpmcore PRIVATE -w -fcommon)
endif()

set_target_properties(cpmcore PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------------
# Qt6 GUI front end
# ---------------------------------------------------------------------------
option(CPM_BUILD_GUI "Build the Qt GUI front end" ON)
# Keep a console attached so the engine's stderr diagnostics are visible while
# developing. Turn OFF for a clean windowed release build.
option(CPM_CONSOLE "Attach a console window (shows engine diagnostics)" ON)
# Copy the Qt runtime and compiler DLLs next to the executable so the build
# folder can be moved to any Windows machine and run without installing Qt.
option(CPM_PORTABLE "Bundle the Qt runtime for a self-contained build folder" ON)

if(CPM_BUILD_GUI)
    # LinguistTools supplies lrelease, used to compile the .qm catalogues that
    # are then embedded into the executable.
    find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
    qt_standard_project_setup()

    qt_add_executable(CPMToolsQt6
        qt/main.cpp
        qt/AppSettings.h     qt/AppSettings.cpp
        qt/Translator.h      qt/Translator.cpp
        qt/CpmBackend.h      qt/CpmBackend.cpp
        qt/MainWindow.h      qt/MainWindow.cpp
        qt/MkfsDialog.h      qt/MkfsDialog.cpp
        qt/AboutDialog.h     qt/AboutDialog.cpp
        qt/FileDropListWidget.h
        qt/FileDropLineEdit.h)

    target_include_directories(CPMToolsQt6 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/qt)

    target_link_libraries(CPMToolsQt6 PRIVATE cpmcore Qt6::Widgets)

    if(MSVC)
        target_compile_definitions(CPMToolsQt6 PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(CPMToolsQt6 PRIVATE -w)
    endif()

    set_target_properties(CPMToolsQt6 PROPERTIES
        WIN32_EXECUTABLE $<NOT:$<BOOL:${CPM_CONSOLE}>>
        MACOSX_BUNDLE OFF)

    # -------------------------------------------------------------------
    # Translations
    #
    # The catalogues are compiled with lrelease and embedded in the binary
    # under /i18n, so no .qm files need to ship beside the executable.
    #
    # lupdate is deliberately not wired into the build: it needs the
    # qt6-declarative tooling (Qt6Qml.dll). Regenerate the .ts files with
    #   python scripts/gen_translations.py
    # -------------------------------------------------------------------
    set(CPM_LANGUAGES en ja zh_CN zh_TW it fr de es)

    # Compile each catalogue with lrelease and embed the result under /i18n.
    # Done with explicit custom commands rather than qt_add_lrelease() so the
    # build works on any Qt 6 minor version that ships LinguistTools.
    set(CPM_QM_FILES "")
    set(_qm_dir "${CMAKE_CURRENT_BINARY_DIR}/i18n")

    foreach(_lang ${CPM_LANGUAGES})
        set(_ts "${CMAKE_CURRENT_SOURCE_DIR}/i18n/cpmtoolsgui_${_lang}.ts")
        set(_qm "${_qm_dir}/cpmtoolsgui_${_lang}.qm")

        add_custom_command(
            OUTPUT  "${_qm}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${_qm_dir}"
            COMMAND Qt6::lrelease "${_ts}" -qm "${_qm}"
            DEPENDS "${_ts}"
            COMMENT "Compiling translation ${_lang}"
            VERBATIM)

        list(APPEND CPM_QM_FILES "${_qm}")
    endforeach()

    qt_add_resources(CPMToolsQt6 "licenseresource"
        PREFIX "/"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

    qt_add_resources(CPMToolsQt6 "translations"
        PREFIX "/i18n"
        BASE "${_qm_dir}"
        FILES ${CPM_QM_FILES})

    # --------------------------------------------------------------- portable
    # The engine reads its disk definitions from beside the executable.
    # configure_file keeps the copy in sync on every (re)configure.
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/diskdefs)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/diskdefs
                       ${CMAKE_CURRENT_BINARY_DIR}/diskdefs COPYONLY)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/diskdefs DESTINATION .)
    endif()

    install(TARGETS CPMToolsQt6 RUNTIME DESTINATION .)

    # -----------------------------------------------------------------------
    # Portable deployment
    #
    # windeployqt copies the Qt6 DLLs, the platform/style/imageformat plugins
    # and (with --compiler-runtime) the MinGW runtime DLLs into the build
    # folder. The resulting directory is self-contained: it can be copied to
    # any 64-bit Windows 10+ machine and run without installing anything.
    #
    # Note the compiler runtime is required even though the GUI is linked
    # dynamically, because the Qt6 DLLs themselves import libstdc++-6.dll and
    # libgcc_s_seh-1.dll.
    # -----------------------------------------------------------------------
    if(CPM_PORTABLE AND NOT MSVC)
        # windeployqt lives beside Qt's other tools; derive it from Qt6_DIR.
        get_filename_component(_qt_prefix "${Qt6_DIR}/../../.." ABSOLUTE)
        find_program(WINDEPLOYQT_EXECUTABLE
            NAMES windeployqt6 windeployqt
            HINTS "${_qt_prefix}/bin"
            DOC "Qt deployment tool")

        if(WINDEPLOYQT_EXECUTABLE)
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(_qwd_config "--debug")
            else()
                set(_qwd_config "--release")
            endif()

            add_custom_command(TARGET CPMToolsQt6 POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                        ${_qwd_config}
                        --no-translations
                        --dir "$<TARGET_FILE_DIR:CPMToolsQt6>"
                        "$<TARGET_FILE:CPMToolsQt6>"
                COMMENT "Deploying Qt libraries and plugins"
                VERBATIM)

            # windeployqt does not pull in the MinGW libraries that the Qt DLLs
            # themselves depend on (libstdc++-6, libgcc_s_seh-1, libicu*,
            # libharfbuzz*, ...). Walk the PE import graph and copy the whole
            # closure so the folder needs nothing installed.
            # Locate PowerShell through its well-known install paths rather than
            # relying on it being on PATH: CMake is often invoked from an MSYS2
            # shell whose PATH does not include the Windows system directories.
            find_program(POWERSHELL_EXECUTABLE
                NAMES powershell.exe pwsh.exe
                HINTS
                    "$ENV{SystemRoot}/System32/WindowsPowerShell/v1.0"
                    "$ENV{ProgramFiles}/PowerShell/7"
                    "$ENV{ProgramW6432}/PowerShell/7"
                DOC "PowerShell interpreter used for runtime deployment")

            get_filename_component(_compiler_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)

            if(NOT CMAKE_OBJDUMP)
                find_program(CMAKE_OBJDUMP NAMES objdump
                    HINTS "${_compiler_bin}")
            endif()

            if(POWERSHELL_EXECUTABLE AND CMAKE_OBJDUMP)
                add_custom_command(TARGET CPMToolsQt6 POST_BUILD
                    COMMAND "${POWERSHELL_EXECUTABLE}"
                            -NoProfile -ExecutionPolicy Bypass
                            -File "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployRuntime.ps1"
                            -TargetDir "$<TARGET_FILE_DIR:CPMToolsQt6>"
                            -SearchDirs "${_qt_prefix}/bin;${_compiler_bin}"
                            -Objdump "${CMAKE_OBJDUMP}"
                    COMMENT "Resolving MinGW runtime DLL dependencies"
                    VERBATIM)
            else()
                message(FATAL_ERROR
                    "Portable build requested (CPM_PORTABLE=ON) but the runtime "
                    "deployment step cannot run.\n"
                    "  PowerShell : '${POWERSHELL_EXECUTABLE}'\n"
                    "  objdump    : '${CMAKE_OBJDUMP}'\n"
                    "Install mingw-w64-ucrt-x86_64-binutils, or configure with "
                    "-DCPM_PORTABLE=OFF to build a non-portable binary.")
            endif()
        else()
            message(FATAL_ERROR
                "Portable build requested (CPM_PORTABLE=ON) but windeployqt was "
                "not found. Install mingw-w64-ucrt-x86_64-qt6-tools, or configure "
                "with -DCPM_PORTABLE=OFF to build a non-portable binary.")
        endif()
    elseif(CPM_PORTABLE AND MSVC)
        message(STATUS
            "CPM_PORTABLE on MSVC: run windeployqt manually or use the "
            "install() rules.")
    endif()
endif()

# ---------------------------------------------------------------------------
# Headless command line harness - lets the engine be exercised from a
# terminal without a display (useful for CI and for scripted checks).
# ---------------------------------------------------------------------------
option(CPM_BUILD_CLI "Build the headless cpmcli test harness" ON)

if(CPM_BUILD_CLI)
    # Built as C++: cpm_test.h uses a bare `extern "C" { }` block.
    add_executable(cpmcli tools/cpmcli.cpp)
    target_link_libraries(cpmcli PRIVATE cpmcore)
    if(MSVC)
        target_compile_definitions(cpmcli PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(cpmcli PRIVATE -w)
        if(CPM_STATIC_RUNTIME)
            # The CLI has no Qt dependency, so it can be a single self-contained
            # file with no libgcc/libstdc++/libwinpthread DLLs required.
            target_link_options(cpmcli PRIVATE
                -static-libgcc -static-libstdc++ -static)
        endif()
    endif()
endif()

# ---------------------------------------------------------------------------
# uishot - renders the main window off-screen to a PNG. A development aid for
# checking the layout without a display; not part of the shipped application.
# ---------------------------------------------------------------------------
option(CPM_BUILD_UISHOT "Build the off-screen UI tooling (uishot, uitest)" OFF)

if(CPM_BUILD_UISHOT AND CPM_BUILD_GUI)
    # Shared source list for the two development tools: they link the real
    # MainWindow so they exercise exactly the code the application runs.
    set(CPM_UI_TOOL_SOURCES
        qt/AppSettings.h     qt/AppSettings.cpp
        qt/Translator.h      qt/Translator.cpp
        qt/CpmBackend.h      qt/CpmBackend.cpp
        qt/MainWindow.h      qt/MainWindow.cpp
        qt/MkfsDialog.h      qt/MkfsDialog.cpp
        qt/AboutDialog.h     qt/AboutDialog.cpp
        qt/FileDropListWidget.h
        qt/FileDropLineEdit.h)

    qt_add_executable(uishot tools/uishot.cpp ${CPM_UI_TOOL_SOURCES})

    target_include_directories(uishot PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/qt)

    target_link_libraries(uishot PRIVATE cpmcore Qt6::Widgets)
    if(NOT MSVC)
        target_compile_options(uishot PRIVATE -w)
    endif()

    # uitest drives the window without a display to verify UI behaviour.
    qt_add_executable(uitest tools/uitest.cpp ${CPM_UI_TOOL_SOURCES})

    target_include_directories(uitest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/qt)

    target_link_libraries(uitest PRIVATE cpmcore Qt6::Widgets)
    if(NOT MSVC)
        target_compile_options(uitest PRIVATE -w)
    endif()

    # Both tools need the resources the application embeds.
    foreach(_tool uishot uitest)
        qt_add_resources(${_tool} "licenseresource"
            PREFIX "/"
            BASE "${CMAKE_CURRENT_SOURCE_DIR}"
            FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

        if(DEFINED CPM_QM_FILES AND CPM_QM_FILES)
            qt_add_resources(${_tool} "translations"
                PREFIX "/i18n"
                BASE "${CMAKE_CURRENT_BINARY_DIR}/i18n"
                FILES ${CPM_QM_FILES})
        endif()
    endforeach()

    # uishot forces the offscreen platform so it can render without a window
    # server; that requires the plugin to sit beside the executable. It is a
    # development-only plugin and is deliberately excluded from the shipped
    # portable package (see cmake/BundleQtRuntime.cmake).
    get_filename_component(_uishot_qt_prefix "${Qt6_DIR}/../../.." ABSOLUTE)
    set(_offscreen_plugin
        "${_uishot_qt_prefix}/share/qt6/plugins/platforms/qoffscreen.dll")

    if(EXISTS "${_offscreen_plugin}")
        add_custom_command(TARGET uishot POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:uishot>/platforms"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_offscreen_plugin}"
                    "$<TARGET_FILE_DIR:uishot>/platforms/qoffscreen.dll"
            COMMENT "Adding the offscreen platform plugin for uishot"
            VERBATIM)
    endif()
endif()

# ---------------------------------------------------------------------------
# Portable ZIP - a folder that runs on any 64-bit Windows 10+ machine with no
# installation and no Qt/MSYS2 present.
# ---------------------------------------------------------------------------
if(WIN32 AND CPM_PORTABLE)
    set(CPM_DIST_NAME "CPMToolsQt6-${CPM_VERSION_TAG}-win64")
    set(CPM_DIST_DIR "${CMAKE_BINARY_DIR}/dist/${CPM_DIST_NAME}")

    add_custom_target(portable
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${CPM_DIST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CPM_DIST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy
                "$<TARGET_FILE:CPMToolsQt6>" "${CPM_DIST_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:cpmcli>" "${CPM_DIST_DIR}/cpmcli.exe"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_BINARY_DIR}/diskdefs" "${CPM_DIST_DIR}/diskdefs"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${CPM_DIST_DIR}/README.md"
        COMMAND ${CMAKE_COMMAND}
                -D "SRC=${CMAKE_BINARY_DIR}"
                -D "DST=${CPM_DIST_DIR}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BundleQtRuntime.cmake"
        COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_BINARY_DIR}/dist"
                ${CMAKE_COMMAND} -E tar cfv "${CPM_DIST_NAME}.zip" --format=zip
                "${CPM_DIST_NAME}"
        DEPENDS CPMToolsQt6 cpmcli
        COMMENT "Creating portable package ${CPM_DIST_NAME}.zip"
        VERBATIM)
endif()
