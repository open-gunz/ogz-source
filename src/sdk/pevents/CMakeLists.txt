add_target(NAME pevents TYPE STATIC SOURCES "src/pevents.cpp")
target_include_directories(pevents INTERFACE src)
target_compile_options(pevents PUBLIC -DWFMO)