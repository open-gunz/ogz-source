file(GLOB src
    "./*.h"
    "./*.cpp"
)

add_target(NAME PatchCreator TYPE EXECUTABLE SOURCES "${src}")

target_include_directories(PatchCreator PUBLIC
	../cml/include
	../launcher/src)

target_link_libraries(PatchCreator PRIVATE
	msvcrt.lib
	msvcmrt.lib
	cml
	launcher_lib
	sodium
	rapidxml)

install(
	TARGETS PatchCreator RUNTIME 
	DESTINATION "tools/"
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)