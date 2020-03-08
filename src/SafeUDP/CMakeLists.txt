file(GLOB src
    "Include/*.h"
    "source/*.cpp"
)

add_target(NAME SafeUDP TYPE STATIC SOURCES "${src}")

target_include_directories(SafeUDP PUBLIC 
	. 
	Include
	../cml/Include
)
target_link_libraries(SafeUDP PUBLIC cml)
