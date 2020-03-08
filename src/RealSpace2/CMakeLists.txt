cmake_policy(SET CMP0015 NEW)

link_directories()

if (UNIX)
set(src Source/RBspObject.cpp Source/RAnimation.cpp Source/RAnimationMgr.cpp Source/RAnimationFile.cpp
	Source/RMath.cpp Source/RAnimationNode.cpp Source/RMeshNodeStringTable.cpp Source/RealSpace2.cpp
	Source/RMeshUtil.cpp Source/RNavigationMesh.cpp Source/RMaterialList.cpp Source/RLenzFlare.cpp
	Source/RSolidBsp.cpp Source/RLightList.cpp Source/RDummyList.cpp Source/ROcclusionList.cpp
	Source/RAStar.cpp Source/LightmapGenerator.cpp Source/RNavigationNode.cpp Source/RToken.cpp)
else()
file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_GDIPLUS")

add_target(NAME RealSpace2 TYPE STATIC SOURCES "${src}")

target_include_directories(RealSpace2 PUBLIC
	.
	Include
	../cml/Include
	../RealSpace/Include
	../sdk)

target_link_libraries(RealSpace2 PUBLIC cml rapidxml)

if (MSVC)
	target_include_directories(RealSpace2 PUBLIC ../sdk/dx9/include)
	find_library(dxerr_path NAMES dxerr PATHS ../sdk/dx9/lib/x86)

	target_link_libraries(RealSpace2 PUBLIC
		${dxerr_path}
		bullet
	)
endif()
