#
# Print a message only if the `VERBOSE_OUTPUT` option is on
#
include(cmake/StandardSettings.cmake)
include(cmake/CCache.cmake)
include(cmake/CPM.cmake)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode.\n")
else()
    message(STATUS "Current config is ${CMAKE_BUILD_TYPE}, not Debug mode, add NDEBUG definition\n")
    add_compile_definitions("NDEBUG")
endif()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

function(verbose_message content)
    if (VERBOSE_OUTPUT)
        message(STATUS ${content})
    endif ()
endfunction()

include(cmake/Conan.cmake)
conan()

function(init_proj)
    cmake_parse_arguments(${CMAKE_CURRENT_FUNCTION} "USE_ALT_NAMES" "" "" "${ARGN}")
    message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")

    #
    # Setup alternative names
    #
    if (${${CMAKE_CURRENT_FUNCTION}_USE_ALT_NAMES})
        string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
        string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)
    endif ()

    #
    # Prevent building in the source directory
    #
    if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
        message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
    endif ()
endfunction()

#
# Create header only library
#
function(config_interface_lib lib_name)
    cmake_parse_arguments(${CMAKE_CURRENT_FUNCTION} "" "STD" "" "${ARGN}")

    add_library(${lib_name} INTERFACE)

    message(STATUS "Added all header files.\n")

    #
    # Set the project standard and warnings
    #
    set(std ${${CMAKE_CURRENT_FUNCTION}_STD})
    if (${std})
        target_compile_features(${lib_name} INTERFACE cxx_std_${std})
        message(STATUS "Using c++ ${std}.\n")
    endif ()

    #
    # Set the build/user include directories
    #
    target_include_directories(
        ${lib_name}
        INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
endfunction()

#
# Create static or shared library, setup header and source files
#
function(config_lib lib_name includes src lib_type)
    cmake_parse_arguments(${CMAKE_CURRENT_FUNCTION} "" "STD" "" "${ARGN}")

    # Find all headers and implementation files
    list(JOIN includes "\n    " includes_str)
    verbose_message("Found the following header files:\n    ${includes_str}")

    list(JOIN src "\n    " src_str)
    verbose_message("Found the following source files:\n    ${src_str}")

    add_library(${lib_name} ${lib_type} ${src})

    if (lib_type STREQUAL "SHARED")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
        set(CMAKE_CXX_VISIBILITY_PRESET hidden)
        set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
    endif ()

    message(STATUS "Added all header and implementation files.\n")

    #
    # Set the project standard and warnings
    #
    set(std ${${CMAKE_CURRENT_FUNCTION}_STD})
    if (${std})
        target_compile_features(${lib_name} PUBLIC cxx_std_${std})
        message(STATUS "Using c++ ${std}.\n")
    endif ()

    #
    # Set the build/user include directories
    #
    target_include_directories(
        ${lib_name}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
endfunction()

#
# Create executable, setup header and source files
#
function(config_exe exe_name exe_src)
    cmake_parse_arguments(${CMAKE_CURRENT_FUNCTION} "" "STD" "INCLUDES;SOURCES" "${ARGN}")
    set(std ${${CMAKE_CURRENT_FUNCTION}_STD})

    list(JOIN exe_src "\n    " exe_src_str)
    verbose_message("Found the following executable source files:\n    ${exe_src_str}")

    add_executable(${exe_name} ${exe_src})

    if (${CMAKE_CURRENT_FUNCTION}_INCLUDES)
        verbose_message("Configuring executable library")

        if (${CMAKE_CURRENT_FUNCTION}_SOURCES)
            config_lib(
                ${exe_name}_LIB
                "${${CMAKE_CURRENT_FUNCTION}_INCLUDES}"
                "${${CMAKE_CURRENT_FUNCTION}_SOURCES}"
                STATIC
                STD ${std}
            )
        else ()
            config_interface_lib(
                ${exe_name}_LIB
                STD ${std}
            )
        endif ()

        target_link_libraries(${exe_name} PUBLIC ${exe_name}_LIB)
    endif ()
endfunction()

#
# Create executable, setup header and source files
#
function(target_install)
    CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")
    packageProject(${ARGN})
endfunction()