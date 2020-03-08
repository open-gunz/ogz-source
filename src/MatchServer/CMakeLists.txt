file(GLOB src
    "*.h"
    "*.cpp"
)

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
list(REMOVE_ITEM src "${main_file}")

add_target(NAME MatchServer_lib TYPE STATIC SOURCES "${src}")

target_include_directories(MatchServer_lib PUBLIC
	.
	../sdk/dx9/Include
	../cml/Include
	../rapidxml/Include
	../CSCommon/Include
	../RealSpace2/Include
	../sdk
	../SafeUDP/Include
	../Locator
	${LIBSQLITE3_INCLUDE_DIR}
	${LIBSODIUM_INCLUDE_DIRS}
)

target_link_libraries(MatchServer_lib PUBLIC
	msvcrt.lib
	msvcmrt.lib
	${ZLIB_LIBRARY}
	sodium
	sqlite3
	rapidxml
	ini
	cml
	CSCommon
	RealSpace2
	SafeUDP
	Locator
)

if (UNIX)
	target_link_libraries(MatchServer_lib PUBLIC dl systemd)
endif()

if (MSVC)
	ucm_add_linker_flags(/SUBSYSTEM:CONSOLE)
	target_link_libraries(MatchServer_lib PUBLIC legacy_stdio_definitions.lib)
endif()

add_target(NAME MatchServer TYPE EXECUTABLE SOURCES "${main_file}")
target_link_libraries(MatchServer PUBLIC MatchServer_lib)

install(
	TARGETS MatchServer RUNTIME 
	DESTINATION "server/"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)