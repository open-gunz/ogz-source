cmake_minimum_required(VERSION 3.0)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../install")

include(macros)

if(MSVC)
	ucm_set_runtime(STATIC)
	ucm_add_flags(/Zi)
	ucm_add_linker_flags(EXE CONFIG Release /DEBUG /OPT:REF /OPT:ICF)

	ucm_add_linker_flags(/ignore:4099 /ignore:4221)
	ucm_add_flags(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
else()
 	ucm_add_flags(-g -fno-strict-aliasing -fno-strict-overflow -fwrapv -DNDEBUG)
 	ucm_add_flags(CXX --std=c++14)
endif()

if (WIN32)
	add_project_subdir(sdk/zlib)
	add_project_subdir(sdk/curl)
	
	set(LIBSODIUM_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/sdk/libsodium/include")
	add_project_subdir(sdk/libsodium)

	set(LIBSQLITE3_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/sdk/sqlite/include")
	add_project_subdir(sdk/sqlite)
else()
	add_project_subdir(sdk/pevents)
	find_package(libcurl)
	find_package(libsqlite)
	find_package(libsodium)
	Include(FindOpenSSL)
endif()

add_project_subdir(sdk/rapidxml)
add_project_subdir(sdk/rapidjson)
add_project_subdir(sdk/ini)

if (WIN32)
	add_project_subdir(sdk/bullet)
	add_project_subdir(sdk/portaudio)
	add_project_subdir(sdk/opus)
	add_project_subdir(sdk/imgui)
endif()


set(UCM_UNITY_BUILD ON)
add_project_subdir(cml)
add_project_subdir(launcher)
add_project_subdir(PatchCreator)
add_project_subdir(CSCommon)
add_project_subdir(RealSpace2)
add_project_subdir(SafeUDP)
add_project_subdir(Locator)
add_project_subdir(MatchServer)
if (WIN32)
	add_project_subdir(Mint2)
	add_project_subdir(RealSound)
	add_project_subdir(Gunz)
	add_project_subdir(PartsIndexer)
endif()
