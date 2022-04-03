file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_WARN_MBCS_MFC_DEPRECATION")

add_target(NAME MDatabase TYPE STATIC SOURCES "${src}")

target_include_directories(MDatabase PUBLIC Include;../cml/Include/)
if (UNIX)
	target_link_libraries(MDatabase PUBLIC odbc)
endif()
