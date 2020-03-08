add_target(NAME sqlite3 TYPE STATIC SOURCES "src/sqlite3.c")
target_include_directories(sqlite3 INTERFACE include)