add_library(portaudio INTERFACE)

target_include_directories(portaudio INTERFACE include)

find_library(portaudio_Release_path NAMES portaudio PATHS lib/Release)

target_link_libraries(portaudio INTERFACE
	${portaudio_Release_path}
)
