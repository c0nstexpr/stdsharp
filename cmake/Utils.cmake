#
# Print a message only if the `VERBOSE_OUTPUT` option is on
#
include(cmake/StandardSettings.cmake)
include(GNUInstallDirs)
include(cmake/Conan.cmake)
include(cmake/Vcpkg.cmake)

set(CPM_DOWNLOAD_VERSION 0.32.1)

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})

function(verbose_message content)
    if(${PROJECT_NAME}_VERBOSE_OUTPUT)
			message(STATUS ${content})
    endif()
endfunction()

#
# Add a target for formating the project using `clang-format` (i.e: cmake --build build --target clang-format)
#

function(list_to_str list str)
    string(REPLACE ";" "\n" str "${list}")
endfunction()

function(target_add_clang_format file_paths)
    if(NOT ${PROJECT_NAME}_CLANG_FORMAT_BINARY)
	    find_program(${PROJECT_NAME}_CLANG_FORMAT_BINARY clang-format)
    endif()

    if(${PROJECT_NAME}_CLANG_FORMAT_BINARY)
		add_custom_target(
            clang-format
			COMMAND ${${PROJECT_NAME}_CLANG_FORMAT_BINARY} -i $${CMAKE_CURRENT_LIST_DIR}/${file_paths}
        )

		message(STATUS "Format using the `clang-format` target (i.e: cmake --build build --target clang-format).\n")
    endif()
endfunction()

function(target_set_warnings target access)
    set(var_prefix target_set_warnings)
    cmake_parse_arguments(${var_prefix} "WARNING_AS_ERROR" "" "" ${ARGN})

    set(MSVC_WARNINGS
        /W4     # Baseline reasonable warnings
        /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss
                # of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to
                # 'type2:field_bits', possible loss of data
        /w14263 # 'function': member function does not override any base class
                # virtual member function
        /w14265 # 'classname': class has virtual functions, but destructor is not
                # virtual instances of this class may not be destructed correctly
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable
                # declared in the for-loop is used outside the for-loop scope
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing
                # an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected
                # operator with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend
                # 'operator'?
        /w14555 # expression has no effect; expected expression with side- effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may
                # cause unexpected runtime behavior.
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined
                # conversion has been implicitly applied
        /permissive- # standards conformance mode for MSVC compiler.
    )
    set(CLANG_WARNINGS
        -Wall
        -Wextra  # reasonable and standard
        -Wshadow # warn the user if a variable declaration shadows one from a
                # parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                            # non-virtual destructor. This helps catch hard to
                            # track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align     # warn for potential performance problem casts
        -Wunused         # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual
                            # function
        -Wpedantic   # warn if non-standard C++ is used
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion  # warn on sign conversions
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output
                    # (ie printf)
    )
    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation # warn if indentation implies blocks where blocks
                                # do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wlogical-op   # warn about logical operations being used where bitwise were
                        # probably wanted
        -Wuseless-cast # warn if you perform a cast to the same type
    )

    if (${${var_prefix}_WARNING_AS_ERROR})
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
        set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
    endif()

    if(MSVC)
        set(warnings ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(warnings ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(warnings ${GCC_WARNINGS})
    else()
        message(AUTHOR_WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif()

    target_compile_options(${target} ${access} ${warnings})
endfunction()

function(init_proj)
    set(var_prefix init_proj)
    cmake_parse_arguments(var_prefix "USE_ALT_NAMES" "" "" ${ARGN})
    message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")

    #
    # Setup alternative names
    #
    if(${${var_prefix}_USE_ALT_NAMES})
	    string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
	    string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)
    else()
	    set(PROJECT_NAME_LOWERCASE ${PROJECT_NAME})
	    set(PROJECT_NAME_UPPERCASE ${PROJECT_NAME})
    endif()

    #
    # Prevent building in the source directory
    #
    if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
      message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
    endif()

    #
    # Enable package managers
    #
    conan()
    vcpkg()
endfunction()

#
# Create header only library
#
function(config_interface_lib lib_name includes)
    set(var_prefix config_interface_lib)
    cmake_parse_arguments(var_prefix "" "STD_VER" "" ${ARGN})
    
    # Find all headers and implementation files
    verbose_message("Found the following header files:")
    list_to_str("${includes}" includes_str)
    verbose_message("${includes_str}")

    add_library(${lib_name} INTERFACE ${includes})

    message(STATUS "Added all header and implementation files.\n")

    #
    # Set the project standard and warnings
    #

    if(${var_prefix}_STD_VER})
        target_compile_features(${lib_name} INTERFACE cxx_std_${std_ver})
    endif()

    target_set_warnings(${lib_name} INTERFACE WARNING_AS_ERROR)
    verbose_message("Applied compiler warnings. Using standard ${CXX_STANDARD}.\n")

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
    set(var_prefix config_interface_lib)
    cmake_parse_arguments(var_prefix "" "STD_VER" "" ${ARGN})

    # Find all headers and implementation files
    verbose_message("Found the following header files:")
    list_to_str("${includes}" includes_str)
    verbose_message("${includes_str}")

    verbose_message("Found the following source files:")
    list_to_str("${src}" src_str)
	verbose_message("${src_str}")

    add_library(${lib_name} ${lib_type} ${includes} ${src})

    if(lib_type STREQUAL "SHARED")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
        set(CMAKE_CXX_VISIBILITY_PRESET hidden)
        set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
    endif()


    message(STATUS "Added all header and implementation files.\n")

    #
    # Set the project standard and warnings
    #

    if(${var_prefix}_STD_VER})
        target_compile_features(${lib_name} PUBLIC cxx_std_${std_ver})
    endif()

    target_set_warnings(${lib_name} PUBLIC WARNING_AS_ERROR)
    verbose_message("Applied compiler warnings. Using standard ${CXX_STANDARD}.\n")

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
    set(var_prefix config_interface_lib)
    cmake_parse_arguments(var_prefix "" "STD_VER" "INCLUDES;SOURCES" ${ARGN})

    if(${${var_prefix}_SOURCES})
        config_lib(${exe_name}_LIB "${${var_prefix}_INCLUDES}" "${${var_prefix}_SOURCES}")
    else()
        config_interface_lib(${exe_name}_LIB "${${var_prefix}_INCLUDES}")
    endif()

    verbose_message("Found the following executable source files:")
    list_to_str("${exe_src}" exe_src_str)
	verbose_message("${exe_src_str}")

    add_executable(${exe_name} ${exe_src})

    target_link_libraries(${exe_name} ${exe_name}_LIB)

    if(${var_prefix}_STD_VER})
        target_compile_features(${exe_name} PUBLIC cxx_std_${std_ver})
    endif()

    target_set_warnings(${exe_name} PUBLIC TRUE)
endfunction()

#
# install target
#
function(target_install target_name ver namespace export_header)
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
      NAMESPACE ${namespace}
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${target_name}
    )

    #
    # Add version header
    #

    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/version.h.in
        include/${target_name}/version.h
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
        DIRECTORY include/${target_name}
        DESTINATION include
    )

    verbose_message("Install targets succesfully build. Install with `cmake --build <build_directory> --target install --config <build_config>`.")

    #
    # Quick `ConfigVersion.cmake` creation
    #

    include(CMakePackageConfigHelpers)

    set(VERSION_FILE_NAME ${target_name}Version.cmake)
    set(CONFIG_FILE_NAME ${target_name}Config.cmake)

    write_basic_package_version_file(
        ${CONFIG_FILE_NAME}
        VERSION ${ver}
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

    #
    # Generate export header if specified
    #

    if(export_header)
        include(GenerateExportHeader)
        generate_export_header(${target_name})
        install(
            FILES ${PROJECT_BINARY_DIR}/${target_name}_export.h 
            DESTINATION include
        )

        message(STATUS "Generated the export header `${target_name}_export.h` and installed it.")
    endif()

    message(STATUS "Finished building requirements for installing the package.\n")
endfunction()
