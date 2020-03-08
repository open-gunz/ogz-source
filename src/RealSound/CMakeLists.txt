file(GLOB src
    "Include/*.h"
    "source/*.cpp"
)

add_target(NAME RealSound TYPE STATIC SOURCES "${src}")

target_include_directories(RealSound PUBLIC
	.
	Include
	../CML/Include
	../RealSpace2/Include
	../sdk/dx9/include
	../sdk/rapidxml/include
	../sdk/rapidjson/include
	../sdk/curl/include
	${OPENSSL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	../sdk/libzip/lib

)