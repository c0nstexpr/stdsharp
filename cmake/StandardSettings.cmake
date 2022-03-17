#
# Package managers
# Currently supporting: Conan, Vcpkg.

option(ENABLE_CONAN "Enable the Conan package manager for this project." OFF)

#
# Miscellaneous options
#
option(VERBOSE_OUTPUT "Enable verbose output, allowing for a better understanding of each step taken." ON)
option(GENERATE_EXPORT_HEADER "Create a `project_export.h` file containing all exported symbols." OFF)