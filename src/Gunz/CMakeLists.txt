cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0015 NEW)

link_directories(
	../Gunz
	../sdk/dx9/lib/x86
	../RealSound/Lib
	../sdk/opus/lib/Release
	../sdk/portaudio/lib/Release)

file(GLOB src
    "*.h"
    "*.cpp"
)

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/WinMain.cpp")
list(REMOVE_ITEM src "${main_file}")

add_target(NAME Gunz_lib TYPE STATIC SOURCES "${src}")

target_include_directories(Gunz_lib
PUBLIC
	../sdk/dx9/include
	../sdk
	../CSCommon/Include
	../SafeUDP/Include
	../CML/Include
	../RealSpace2/Include
	../MAI/Include
	../RealSpace/Include
	../Mint2/Include
	../RealSound/Include
	../MCountryFilter/Include
	../sdk/rapidxml/include
	${LIBSODIUM_INCLUDE_DIRS}
	../sdk/opus/include
	../sdk/portaudio/include
INTERFACE
    .
)

find_library(fmodvc_path NAMES fmodvc PATHS .)

target_link_libraries(Gunz_lib PUBLIC
    msvcrt.lib
	msvcmrt.lib
	legacy_stdio_definitions.lib
	winmm.lib
	${ZLIB_LIBRARY}
	${fmodvc_path}
	sodium
	opus
	bullet
    portaudio

	cml
	RealSpace2
	CSCommon
	Mint2
	RealSound
	SafeUDP
)

target_compile_options(Gunz_lib
    PUBLIC -D_LOCATOR
    PRIVATE -DLIB_COMPILE)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(Gunz_lib PUBLIC
		/wd4244 /wd4305)
	ucm_add_linker_flags(/SUBSYSTEM:WINDOWS)
endif()

add_target(NAME Gunz TYPE EXECUTABLE SOURCES "${main_file};${CMAKE_CURRENT_SOURCE_DIR}/Gunz.rc")
set_target_properties(Gunz PROPERTIES COMPILE_DEFINITIONS BUILDER_STATIC_DEFINE)
target_link_libraries(Gunz PUBLIC Gunz_lib)

install(
	TARGETS Gunz RUNTIME 
	DESTINATION "client/"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
install(
	FILES $<TARGET_PDB_FILE:Gunz> 
	RENAME 
	DESTINATION "pdb/" 
	OPTIONAL
)