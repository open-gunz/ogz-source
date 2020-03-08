add_library(opus INTERFACE)

target_include_directories(opus INTERFACE include)

find_library(celt_path NAMES celt PATHS lib/Release)
find_library(opus_path NAMES opus PATHS lib/Release)
find_library(silk_common_path NAMES silk_common PATHS lib/Release)
find_library(silk_fixed_path NAMES silk_fixed PATHS lib/Release)
find_library(silk_float_path NAMES silk_float PATHS lib/Release)

target_link_libraries(opus INTERFACE
	${celt_path}
	${opus_path}
	${silk_common_path}
	${silk_fixed_path}
	${silk_float_path}
)