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
    cmake_parse_arguments(ARG "USE_ALT_NAMES" "" "" ${ARGN})
    message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")

    #
    # Setup alternative names
    #
    if (${ARG_USE_ALT_NAMES})
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

function(target_include_as_system target_name lib_type)
    get_target_property(included ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${target_name} SYSTEM BEFORE ${lib_type} ${included})
endfunction()

#
# Create header only library
#
function(config_interface_lib lib_name)
    cmake_parse_arguments(ARG "" "STD" "" ${ARGN})

    add_library(${lib_name} INTERFACE)

    message(STATUS "Added all header files.\n")

    #
    # Set the project standard and warnings
    #
    set(std ${ARG_STD})
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
    cmake_parse_arguments(ARG "" "STD" "" ${ARGN})

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
    set(std ${ARG_STD})
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
    cmake_parse_arguments(ARG "" "STD" "INCLUDES;SOURCES" ${ARGN})
    set(std ${ARG_STD})

    list(JOIN exe_src "\n    " exe_src_str)
    verbose_message("Found the following executable source files:\n    ${exe_src_str}")

    add_executable(${exe_name} ${exe_src})

    if (ARG_INCLUDES)
        verbose_message("Configuring executable library")

       if (ARG_SOURCES)
            config_lib(
                ${exe_name}_LIB
                "${ARG_INCLUDES}"
                "${ARG_SOURCES}"
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

function(target_enable_coverage target_name)
    message(STATUS "enable coverage for ${target_name}")
    cmake_parse_arguments(ARG "" "FORMAT" "" ${ARGN})

    set(options -fprofile-instr-generate -fcoverage-mapping)

    target_compile_options(${target_name} PUBLIC ${options})
    target_link_options(${target_name} PUBLIC ${options})

    get_filename_component(llvm_bin ${CMAKE_CXX_COMPILER} DIRECTORY)
    set(file_profdata_name ${target_name}.profdata)

    add_custom_command(
        OUTPUT ${file_profdata_name}
        COMMAND ${llvm_bin}/llvm-profdata
        ARGS merge -sparse $<TARGET_NAME_IF_EXISTS:${target_name}>.profraw
    )

    if(${ARG_FORMAT} STREQUAL text)
        set(coverage_file coverage.json)
    elseif(${ARG_FORMAT} STREQUAL html)
        set(coverage_file coverage.html)
    elseif(${ARG_FORMAT} STREQUAL lcov)
        set(coverage_file coverage.lcov)
    else()
        message(WARNING "unknown format ${ARG_FORMAT}")
        set(coverage_file coverage.${ARG_FORMAT})
    endif()

    add_custom_command(
        OUTPUT ${coverage_file}
        DEPENDS ${file_profdata_name}
        COMMAND ${llvm_bin}/llvm-cov
        ARGS export --format=text --object=$<TARGET_FILE:${target_name}> --instr-profile=${file_profdata_name} > ${coverage_file}
        VERBATIM
        USES_TERMINAL
    )

    add_custom_target(coverage DEPENDS ${coverage_file})
endfunction()