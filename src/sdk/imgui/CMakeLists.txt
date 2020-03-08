file(GLOB src
    "./*.h"
    "./*.cpp"
)

add_target(NAME imgui TYPE STATIC SOURCES "${src}")

target_include_directories(imgui INTERFACE "./")
