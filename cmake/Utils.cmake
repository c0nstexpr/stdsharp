option(VERBOSE_OUTPUT "Enable verbose output, allowing for a better understanding of each step taken." ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode.\n")
else()
    message(STATUS "Current config is ${CMAKE_BUILD_TYPE}, not Debug mode, add NDEBUG definition\n")
    add_compile_definitions("NDEBUG")
endif()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

function(verbose_message)
    if (VERBOSE_OUTPUT)
        message(STATUS ${ARGN})
    endif ()
endfunction()

function(target_include_as_system target_name)
    get_target_property(included ${target_name} INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(target_type ${target_name} TYPE)
    target_include_directories(${target_name} SYSTEM BEFORE ${target_type} ${included})
endfunction()

#
# Create static or shared library, setup header and source files
#
function(config_lib lib_name lib_type)
    cmake_parse_arguments(ARG "" "STD;VER" "INC_DIR;INSTALL_INC_DIR;SRC" ${ARGN})

    if(NOT DEFINED ARG_INC_DIR)
        set(ARG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    endif ()

    if(NOT DEFINED INSTALL_INC_DIR)
        set(INSTALL_INC_DIR include)
    endif ()

    list(JOIN ARG_INC_DIR "\n    " includes_str)
    verbose_message("Found the following include dir:\n    ${includes_str}")

    list(JOIN ARG_SRC "\n    " src_str)
    verbose_message("Found the following source files:\n    ${src_str}")

    add_library(${lib_name} ${lib_type} ${ARG_SRC})

    set(inc_tag PUBLIC)

    if (lib_type STREQUAL "SHARED")
        set_target_properties(${lib_name} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS OFF)
    else(lib_type STREQUAL "INTERFACE")
        set(inc_tag INTERFACE)
    endif ()

    if (ARG_STD)
        target_compile_features(${lib_name} ${inc_tag} cxx_std_${ARG_STD})
        message(STATUS "Using c++ ${ARG_STD}.\n")
    endif ()

    target_include_directories(
        ${lib_name}
        ${inc_tag}
            $<INSTALL_INTERFACE:${ARG_INSTALL_INC_DIR}>
            $<BUILD_INTERFACE:${ARG_INC_DIR}>
    )

    if(NOT DEFINED ARG_VER)
        set(ARG_VER ${CMAKE_PROJECT_VERSION})
    endif ()

    set_target_properties(${lib_name} PROPERTIES VERSION ${ARG_VER})
endfunction()

#
# Create executable, setup header and source files
#
function(config_exe exe_name)
    cmake_parse_arguments(ARG "" "STD;VER" "EXE_SRC" ${ARGN})

    list(JOIN ARG_EXE_SRC "\n    " exe_src_str)
    verbose_message("Found the following executable source files:\n    ${exe_src_str}")

    add_executable(${exe_name} ${ARG_EXE_SRC})

    if(NOT DEFINED ARG_VER)
        set(ARG_VER ${CMAKE_PROJECT_VERSION})
    endif ()

    set_target_properties(${exe_name} PROPERTIES VERSION ${ARG_VER})

    if (ARG_STD)
        target_compile_features(${exe_name} PUBLIC cxx_std_${ARG_STD})
        message(STATUS "Using c++ ${ARG_STD}.\n")
    endif ()
endfunction()

#
# Create executable, setup header and source files
#
function(target_install target)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    cmake_parse_arguments(ARG "ARCH_INDEPENDENT" "BIN_DIR;INC_DST;VER;COMPATIBILITY;NAMESPACE;CONFIG_FILE" "DEPENDENCIES" ${ARGN})

    get_target_property(target_type ${target} TYPE)

    if(NOT DEFINED ARG_BIN_DIR)
        get_target_property(ARG_BIN_DIR ${target} BINARY_DIR)
        verbose_message("Use default binary dir ${ARG_BIN_DIR}")
    endif ()

    if(NOT DEFINED ARG_INC_DST)
        set(ARG_INC_DST "./")
    endif ()

    if(NOT DEFINED ARG_VER)
        get_target_property(ARG_VER ${target} VERSION)
        verbose_message("Use default version ${ARG_VER}")
    endif ()

    set(ARG_VER_SUFFIX "-${ARG_VER}")

    install(
        TARGETS ${target}
        EXPORT ${target}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${target}
                COMPONENT "${target}_Runtime"
                NAMELINK_COMPONENT "${target}_Development"
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${target}
                COMPONENT "${target}_Development"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${target}
                COMPONENT "${target}_Runtime"
        BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}/${target}
               COMPONENT "${target}_Runtime"
        PUBLIC_HEADER DESTINATION ${ARG_INC_DST} COMPONENT "${target}_Development"
        INCLUDES DESTINATION "${ARG_INC_DST}"
    )

    if((NOT DEFINED ARG_ARCH_INDEPENDENT) AND (target_type STREQUAL "INTERFACE_LIBRARY"))
        set(ARG_ARCH_INDEPENDENT YES)
    endif()

    set(ARG_CMAKE_DIR "${target}${ARG_VER_SUFFIX}")

    verbose_message("CMake files directory: ${ARG_CMAKE_DIR}")

    set(
        INSTALL_CMAKEDIR
        "${CMAKE_INSTALL_LIBDIR}/cmake/${ARG_CMAKE_DIR}"
        CACHE PATH "CMake package config location relative to the install prefix"
    )

    verbose_message("CMake files install directory: ${INSTALL_CMAKEDIR}")

    install(
        EXPORT ${target}Targets
        DESTINATION ${INSTALL_CMAKEDIR}
        NAMESPACE ${ARG_NAMESPACE}
        COMPONENT "${target}_Development"
    )

    if(ARG_ARCH_INDEPENDENT)
        set(wbpvf_extra_args ARCH_INDEPENDENT)
    endif()

    set(version_config "${ARG_BIN_DIR}/${target}ConfigVersion.cmake")
    set(target_config "${ARG_BIN_DIR}/${target}Config.cmake")

    write_basic_package_version_file(
        "${version_config}"
        VERSION ${ARG_VER}
        COMPATIBILITY ${ARG_COMPATIBILITY} ${wbpvf_extra_args}
    )

    if(ARG_CONFIG_FILE)
        configure_file("${ARG_CONFIG_FILE}" "${target_config}" @ONLY)
    else()
        file(
            CONFIGURE OUTPUT "${target_config}"
            CONTENT "
                include(CMakeFindDependencyMacro)

                string(REGEX MATCHALL \"[^;]+\" SEPARATE_DEPENDENCIES \"@ARG_DEPENDENCIES@\")

                foreach(dependency ${SEPARATE_DEPENDENCIES})
                    string(REPLACE \" \" \";\" args \"${dependency}\")
                    find_dependency(${args})
                endforeach()

                include(\"${CMAKE_CURRENT_LIST_DIR}/@target@Targets.cmake\")
            "
        )
    endif()

    install(
        FILES "${version_config}" "${target_config}"
        DESTINATION ${INSTALL_CMAKEDIR}
        COMPONENT "${target}_Development"
    )

    get_target_property(target_included ${target} INTERFACE_INCLUDE_DIRECTORIES)

    install(
        DIRECTORY ${target_included}
        DESTINATION ${ARG_INC_DST}
        COMPONENT "${target}_Development"
    )
endfunction()

function(target_enable_clang_tidy target)
    find_program(CLANG_TIDY clang-tidy)
    if(CLANG_TIDY)
        message(STATUS "found clang-tidy: ${CLANG_TIDY}")
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY}")
    else()
        message(STATUS "clang-tidy not found")
    endif()
endfunction(target_enable_clang_tidy)
