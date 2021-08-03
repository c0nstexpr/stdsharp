#
# Print a message only if the `VERBOSE_OUTPUT` option is on
#
include(cmake/StandardSettings.cmake)
include(cmake/CPM.cmake)
include(cmake/CCache.cmake)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

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
    set(PROJECT_NAME_LOWERCASE ${PROJECT_NAME} PARENT_SCOPE)
    set(PROJECT_NAME_UPPERCASE ${PROJECT_NAME} PARENT_SCOPE)
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
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
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
# install target
#
function(target_install target_name)
    include(GNUInstallDirs)

    cmake_parse_arguments(${CMAKE_CURRENT_FUNCTION} "" "VER;NAMESPACE" "" "${ARGN}")

    install(
        TARGETS ${target_name}
        EXPORT ${target_name}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION include
        PUBLIC_HEADER DESTINATION include
    )

    install(
        EXPORT ${target_name}Targets
        FILE ${target_name}Targets.cmake
        NAMESPACE ${${CMAKE_CURRENT_FUNCTION}_NAMESPACE}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${target_name}
    )

    #
    # Add version header
    #
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/version.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/${target_name}/version.h
        @ONLY
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/include/${target_name}/version.h
        DESTINATION include/${target_name}
    )

    #
    # Install the `include` directory
    #
    install(
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${target_name}
        DESTINATION include
    )

    verbose_message("Install targets succesfully build. Install with `cmake --build <build_directory> --target install --config <build_config>`.")

    #
    # Quick `ConfigVersion.cmake` creation
    #
    set(VERSION_FILE_NAME ${target_name}Version.cmake)
    set(CONFIG_FILE_NAME ${target_name}Config.cmake)

    write_basic_package_version_file(
        ${CONFIG_FILE_NAME}
        VERSION ${${CMAKE_CURRENT_FUNCTION}_VER}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${CMAKE_SOURCE_DIR}/cmake/Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${target_name}
    )

    message(STATUS "Finished building requirements for installing the package.\n")
endfunction()

#
# Generate export header
#
function(target_export_header target_name)
    cmake_parse_arguments(${CMAKE_CURRENT_FUNCTION} "INSTALL" "" "" ${ARGN})

    generate_export_header(${target_name})

    message(STATUS "Generated the export header `${target_name}_export.h`")

    if (${${CMAKE_CURRENT_FUNCTION}_INSTALL})
        install(
            FILES ${PROJECT_BINARY_DIR}/${target_name}_export.h
            DESTINATION include
        )
    endif ()
endfunction()
