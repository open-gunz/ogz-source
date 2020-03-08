# - Try to find LibCURL headers and libraries.
#
# Usage of this module as follows:
#
#     find_package(LibCURL)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  LibCURL_ROOT_DIR  Set this variable to the root installation of
#                    LibCURL if the module has problems finding
#                    the proper installation path.
#
# Variables defined by this module:
#
#  LIBCURL_FOUND              System has LibCURL libs/headers
#  LibCURL_LIBRARIES          The LibCURL libraries
#  LibCURL_INCLUDE_DIR        The location of LibCURL headers

set(LibCURL_ROOT_DIR /usr/local/opt/curl/)

find_path(LibCURL_ROOT_DIR
    NAMES include/curl/curl.h
)

find_library(LibCURL_LIBRARIES
    NAMES curl
    HINTS ${LibCURL_ROOT_DIR}/lib
)

find_path(LibCURL_INCLUDE_DIR
    NAMES curl/curl.h
    HINTS ${LibCURL_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibCURL DEFAULT_MSG
    LibCURL_LIBRARIES
    LibCURL_INCLUDE_DIR
)

mark_as_advanced(
    LibCURL_ROOT_DIR
    LibCURL_LIBRARIES
    LibCURL_INCLUDE_DIR
)
