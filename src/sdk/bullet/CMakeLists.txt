add_library(bullet INTERFACE)

find_library(BulletCollision_Debug_path NAMES BulletCollision PATHS lib/Debug)
find_library(LinearMath_Debug_path NAMES LinearMath PATHS lib/Debug)

target_link_libraries(bullet INTERFACE debug
	${BulletCollision_Debug_path}
	${LinearMath_Debug_path})

find_library(BulletCollision_Release_path NAMES BulletCollision PATHS lib/Release)
find_library(LinearMath_Release_path NAMES LinearMath PATHS lib/Release)

target_link_libraries(bullet INTERFACE optimized
	${BulletCollision_Release_path}
	${LinearMath_Release_path})