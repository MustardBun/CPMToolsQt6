# Gathers the DLLs and Qt plugins that windeployqt placed in the build folder
# and copies them into the portable distribution directory.
#
# Invoked from the `portable` custom target with:
#   -DSRC=<build dir>  -DDST=<dist dir>

if(NOT SRC OR NOT DST)
    message(FATAL_ERROR "BundleQtRuntime.cmake: SRC and DST are required")
endif()

# Qt puts plugins in subdirectories; these are the ones a Widgets application
# actually needs. Missing optional directories (e.g. imageformats) are ignored.
set(_plugin_dirs
    platforms
    styles
    imageformats
    iconengines
    tls
    networkinformation
    generic)

# Everything windeployqt copied to the top level: Qt6*.dll plus the MinGW
# compiler runtime.
file(GLOB _top_level "${SRC}/*.dll")

set(_copied 0)

foreach(_dll ${_top_level})
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_dll}" "${DST}/"
        RESULT_VARIABLE _rc)
    if(_rc EQUAL 0)
        math(EXPR _copied "${_copied} + 1")
        get_filename_component(_name "${_dll}" NAME)
        message(STATUS "  bundled ${_name}")
    else()
        message(WARNING "  failed to copy ${_dll}")
    endif()
endforeach()

foreach(_dir ${_plugin_dirs})
    if(EXISTS "${SRC}/${_dir}")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                    "${SRC}/${_dir}" "${DST}/${_dir}"
            RESULT_VARIABLE _rc)
        if(_rc EQUAL 0)
            message(STATUS "  bundled plugin folder ${_dir}/")
        else()
            message(WARNING "  failed to copy plugin folder ${_dir}/")
        endif()
    endif()
endforeach()

# The offscreen plugin is a development aid used by the uishot target; the
# shipped application never needs it.
set(_dev_only_plugins "platforms/qoffscreen.dll")
foreach(_dev_plugin ${_dev_only_plugins})
    if(EXISTS "${DST}/${_dev_plugin}")
        file(REMOVE "${DST}/${_dev_plugin}")
        message(STATUS "  excluded development-only plugin ${_dev_plugin}")
    endif()
endforeach()

if(_copied EQUAL 0)
    message(WARNING
        "No DLLs found in ${SRC}. Was windeployqt run? The package will not be "
        "portable. Reconfigure with -DCPM_PORTABLE=ON before building.")
else()
    message(STATUS "Bundled ${_copied} DLL(s) into the portable package.")
endif()
