file(GLOB src
    "*.h"
    "*.cpp"
)

add_target(NAME Locator TYPE STATIC SOURCES "${src}")

target_include_directories(Locator PRIVATE
	../MatchServer
	../RealSpace2/Include
	../SafeUDP/Include
	../MCountryFilter/Include
	${LIBSQLITE3_INCLUDE_DIR}
	../sdk/dx9/include
	../sdk)

target_link_libraries(Locator PUBLIC
	ini
	cml
	CSCommon
	SafeUDP
)
	
