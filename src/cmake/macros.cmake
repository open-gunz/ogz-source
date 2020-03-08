include(ucm)

macro(add_target)
    #cmake_parse_arguments(ARG "UNITY" "NAME;TYPE;PCH_FILE;CPP_PER_UNITY" "UNITY_EXCLUDED;SOURCES" ${ARGN})
	
	ucm_set_runtime(STATIC)
	
	if (${UCM_UNITY_BUILD})
		set(MACROS_FLAGS UNITY CPP_PER_UNITY 500)
	endif()
	
	ucm_add_target(${ARGV} ${MACROS_FLAGS})
	
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_PUBLISH")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -D_PUBLISH")
	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -D_PUBLISH")

	set(projects_added "${projects_added};${ARG_NAME}" CACHE INTERNAL "projects_added")
endmacro()

macro(add_project_subdir name)
	set(projects_added "" CACHE INTERNAL "projects_added")

	add_subdirectory(${name})
	
	foreach(target ${projects_added})
		if (TARGET ${target}_ORIGINAL)
			get_property(INCLUDE_DIRS TARGET ${target} PROPERTY INCLUDE_DIRECTORIES)
			if(NOT "${INCLUDE_DIRS}" STREQUAL "")
				target_include_directories(${target}_ORIGINAL PRIVATE ${INCLUDE_DIRS})
				get_property(INCLUDE_DIRS2 TARGET ${target}_ORIGINAL PROPERTY INCLUDE_DIRECTORIES)
			endif()
		endif()
	endforeach()
endmacro()