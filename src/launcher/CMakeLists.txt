file(GLOB src
    "src/*.h"
    "src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../cml/Include/*.h"
)

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM src "${main_file}")
set(imgui_impl_dx9 "${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_dx9.cpp")
list(REMOVE_ITEM src "${imgui_impl_dx9}")

add_target(NAME launcher_lib TYPE STATIC SOURCES "${src}")

target_include_directories(launcher_lib PUBLIC
	src
	"${CMAKE_CURRENT_SOURCE_DIR}/../cml/Include/*.h"
)

target_link_libraries(launcher_lib PUBLIC msvcrt.lib msvcmrt.lib cml curl sodium)

if (WIN32)
	add_target(NAME launcher TYPE EXECUTABLE SOURCES "${main_file}" "${imgui_impl_dx9}")
	target_link_libraries(launcher PUBLIC launcher_lib imgui)

	if(MSVC)
		set_target_properties(launcher PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS")
	endif()

	install(
		TARGETS launcher RUNTIME 
		DESTINATION "client/"
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	)
endif()
