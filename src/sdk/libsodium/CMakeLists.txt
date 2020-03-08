file(GLOB_RECURSE src
    "src/*.h"
    "src/*.c"
)

add_target(NAME sodium TYPE STATIC SOURCES "${src}")

target_include_directories(sodium
	INTERFACE include
	PRIVATE include/sodium)

target_compile_options(sodium PUBLIC -DSODIUM_STATIC)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(sodium PRIVATE
		/wd4146 # Unary minus applied to unsigned type, result still unsigned
		/wd4197 # Top-level volatile in cast is ignored
		/wd4244 # Narrowing implicit cast
		/wd4715 # Not all control paths return a value
	)
endif()