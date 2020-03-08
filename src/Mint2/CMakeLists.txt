file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

add_target(NAME Mint2 TYPE STATIC SOURCES "${src}")

target_include_directories(Mint2 PUBLIC
	.
	Include)

target_link_libraries(Mint2 PUBLIC cml)