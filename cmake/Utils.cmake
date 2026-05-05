set(CMAKE_COLOR_DIAGNOSTICS ON)

function(target_diagnostics target_name)
    target_compile_options(
        ${target_name} PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-fdiagnostics-show-template-tree>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8,/diagnostics:caret>
    )
endfunction()

function(target_set_common_cxx_properties target_name target_tag)
    cmake_parse_arguments(ARG "" "STD;VER;INCLUDED_AS_SYSTEM" "INC_DIR;SRC;LINKS" ${ARGN})

    if(DEFINED ARG_STD)
        message(STATUS "Using c++ ${ARG_STD}")
        target_compile_features(${target_name} ${target_tag} cxx_std_${ARG_STD})
    endif()

    if(NOT DEFINED ARG_VER)
        set(ARG_VER "${PROJECT_VERSION}")
    endif()
    message(STATUS "Setting version to ${ARG_VER}")

    set_target_properties(
        ${target_name} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
        VERSION "${ARG_VER}"
        CXX_EXTENSIONS OFF
        EXPORT_COMPILE_COMMANDS ON
    )
    if(NOT DEFINED ARG_INC_DIR)
        set(ARG_INC_DIR include)
    endif()

    if((NOT DEFINED ARG_INCLUDED_AS_SYSTEM) AND (NOT PROJECT_IS_TOP_LEVEL))
        set(ARG_INCLUDED_AS_SYSTEM TRUE)
    endif()

    if(ARG_INCLUDED_AS_SYSTEM)
        message(STATUS "Include directories will be marked as SYSTEM")
        set(system_inc_tag SYSTEM)
    endif()

    message(STATUS "Found the following include dir:")
    foreach(inc_dir ${ARG_INC_DIR})
        message(STATUS "${inc_dir}")
    endforeach()

    message(STATUS "Found the following source files:")
    foreach(src ${ARG_SRC})
        message(STATUS "${src}")
    endforeach()

    target_include_directories(
        ${target_name} ${system_inc_tag} ${target_tag}
        $<INSTALL_INTERFACE:${ARG_INC_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${ARG_INC_DIR}>
    )
    if(DEFINED ARG_SRC)
        target_sources(${target_name} PRIVATE ${ARG_SRC})
    endif()

    message(STATUS "Linking with the following libraries: ${ARG_LINKS}")
    target_link_libraries(${target_name} ${target_tag} ${ARG_LINKS})
endfunction()

# Create static or shared library, setup header and source files
function(add_interface_target target_name)
    message(STATUS "Creating interface library ${target_name}")
    add_library(${target_name} INTERFACE)
    target_set_common_cxx_properties(${target_name} INTERFACE ${ARGN})
endfunction()

# Create static or shared library, setup header and source files
function(add_lib_target target_name lib_type)
    message(STATUS "Creating ${lib_type} library ${target_name}")
    add_library(${target_name} ${lib_type})
    target_set_common_cxx_properties(${target_name} PRIVATE ${ARGN})
endfunction()

# Create executable, setup header and source files
function(add_exe_target target_name)
    message(STATUS "Creating executable ${target_name}")
    add_executable(${target_name})
    target_set_common_cxx_properties(${target_name} PRIVATE ${ARGN})
endfunction()

# install targets in project
function(project_install)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    cmake_parse_arguments(ARG "" "NAMESPACE;COMPATIBILITY;CONFIG_FILE" "TARGETS" ${ARGN})
    if(NOT DEFINED ARG_TARGETS)
        set(ARG_TARGETS ${PROJECT_NAME})
    endif()

    if(NOT DEFINED ARG_NAMESPACE)
        set(ARG_NAMESPACE ${PROJECT_NAME})
    endif()

    if(NOT DEFINED ARG_CONFIG_FILE)
        set(ARG_CONFIG_FILE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Config.cmake.in")
        message(STATUS "Using default config file: ${ARG_CONFIG_FILE}")
    endif()

    set(PROJECT_INSTALL_CMAKEDIR "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}")
    message(STATUS "CMake files install directory: ${PROJECT_INSTALL_CMAKEDIR}")
    # Declare the install components for the target.
    install(TARGETS ${ARG_TARGETS} EXPORT ${PROJECT_NAME}Targets FILE_SET HEADERS)
    # Install the exported target
    install(
        EXPORT ${PROJECT_NAME}Targets
        DESTINATION "${PROJECT_INSTALL_CMAKEDIR}"
        NAMESPACE ${ARG_NAMESPACE}::
    )


    set(version_config "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
    write_basic_package_version_file(
        "${version_config}"
        COMPATIBILITY ${ARG_COMPATIBILITY}
    )

    set(project_config "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
    configure_package_config_file(
        ${ARG_CONFIG_FILE} ${project_config}
        INSTALL_DESTINATION "${PROJECT_INSTALL_CMAKEDIR}"
    )

    install(
        FILES "${version_config}" "${project_config}"
        DESTINATION "${PROJECT_INSTALL_CMAKEDIR}"
    )

    return(PROPAGATE PROJECT_INSTALL_CMAKEDIR)
endfunction()

function(target_clang_tidy target_name)
    message(CHECK_START "Checking for clang-tidy")
    find_program(CLANG_TIDY clang-tidy)
    if(NOT EXISTS "${CLANG_TIDY}")
        message(CHECK_FAIL "clang-tidy not found")
        return()
    endif()

    message(CHECK_PASS "found clang-tidy: ${CLANG_TIDY}")
    cmake_parse_arguments(ARG "" "ENABLE_PROFILE" "" ${ARGN})

    if(ARG_ENABLE_PROFILE)
        get_target_property(bin_dir ${target_name} BINARY_DIR)
        set(report_folder "${bin_dir}/clang-tidy-report")

        add_custom_target(
            ${target_name}ClangTidyClean
            ALL
            COMMAND "${CMAKE_COMMAND}" -E rm ${report_folder}
            USES_TERMINAL
        )

        add_dependencies(${target_name} ${target_name}ClangTidyClean)

        list(
            APPEND
            CLANG_TIDY
            "--enable-check-profile"
            "--store-check-profile=${report_folder}"
        )
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        list(APPEND CLANG_TIDY "--extra-arg=-EHsc") # TODO: https://github.com/llvm/llvm-project/issues/44701
    endif()

    set_target_properties(
        ${target_name}
        PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY}"
    )
endfunction()

function(target_clang_sanitizer target_name)
    get_target_property(compiler ${target_name} CXX_COMPILER_ID)
    if(NOT compiler MATCHES "(Apple)?[Cc]lang")
        return()
    endif()

    cmake_parse_arguments(ARG "" "" "SANITIZER" ${ARGN})
    foreach(sanitizer ${ARG_SANITIZER})
        set(sanitizer_options "${sanitizer_options} -fsanitize=${sanitizer}")
    endforeach()

    target_compile_options(
        ${target_name} PRIVATE
        "${sanitizer_options} -fno-omit-frame-pointer -fno-optimize-sibling-calls -g"
    )
    target_link_options(${target_name} PRIVATE "${sanitizer_options}")
endfunction()

function(target_msvc_sanitizer target_name)
    get_target_property(compiler ${target_name} CXX_COMPILER_ID)
    if(NOT compiler MATCHES "MSVC")
        return()
    endif()

    cmake_parse_arguments(ARG "" "" "SANITIZER" ${ARGN})
    foreach(sanitizer ${ARG_SANITIZER})
        set(sanitizer_options "${sanitizer_options} /fsanitize=${sanitizer}")
    endforeach()

    target_compile_options(
        ${target_name} PRIVATE
        "${sanitizer_options} /fsanitize-address-use-after-return /Zi"
    )
    target_link_options(${target_name} PRIVATE "${sanitizer_options}")
endfunction()

function(target_llvm_coverage target_name)
    get_target_property(compiler ${target_name} CXX_COMPILER_ID)
    if(NOT compiler MATCHES "(Apple)?[Cc]lang")
        return()
    endif()

    if(NOT DEFINED ARG_FORMAT)
        set(ARG_FORMAT lcov)
    endif()

    if(${ARG_FORMAT} STREQUAL text)
        set(coverage_file_ext json)
    elseif(${ARG_FORMAT} STREQUAL lcov)
        set(coverage_file_ext lcov)
    else()
        message(FATAL_ERROR "unknown format ${ARG_FORMAT}")
    endif()

    message(CHECK_START "Checking for llvm-profdata and llvm-cov")
    find_program(llvm_profdata "llvm-profdata")
    find_program(llvm_cov "llvm-cov")
    if(NOT EXISTS "${llvm_profdata}" OR NOT EXISTS "${llvm_cov}")
        message(CHECK_FAIL "llvm-profdata or llvm-cov not found")
        return()
    endif()

    message(CHECK_PASS "found llvm-profdata at: ${llvm_profdata}")
    message(CHECK_PASS "found llvm-cov at: ${llvm_cov}")
    message(STATUS "enable code coverage for ${target_name}")
    cmake_parse_arguments(ARG "" "FORMAT" "DEPENDS" ${ARGN})

    set(profile_file "${target_name}.profraw")

    # Run first to generate profraw file
    add_custom_command(
        OUTPUT ${profile_file}
        DEPENDS ${target_name}
        COMMAND
        "${CMAKE_COMMAND}" -E env LLVM_PROFILE_FILE=${profile_file}
        $<TARGET_FILE:${target_name}> || exit 0
    )

    set(options -fprofile-instr-generate -fcoverage-mapping)
    target_compile_options(${target_name} PRIVATE ${options})
    target_link_options(${target_name} PRIVATE ${options})

    set(profdata_file_name "${target_name}.profdata")
    add_custom_target(
        ${target_name}CoverageReport
        ALL
        DEPENDS ${profile_file}
        COMMAND
        "${llvm_profdata}" merge --sparse -o="${profdata_file_name}"
        "${profile_file}"
        COMMAND
        "${llvm_cov}" export -format=${ARG_FORMAT}
        -object="$<TARGET_FILE:${target_name}>"
        -instr-profile="${profdata_file_name}" > "${target_name}Coverage.${coverage_file_ext}"
        USES_TERMINAL
    )
endfunction()
