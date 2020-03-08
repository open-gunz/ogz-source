file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

add_target(NAME CSCommon TYPE STATIC SOURCES "${src}")

target_include_directories(CSCommon PUBLIC
	.
	Include
	../SafeUDP/Include
	../RealSpace2/Include
	../sdk/dx9/Include
	../sdk
	../MCountryFilter/Include
	../sdk/rapidxml/include
	../cml/Include
	${LIBSODIUM_INCLUDE_DIRS}
)

target_link_libraries(CSCommon PUBLIC
	cml
	rapidxml
	sodium)
