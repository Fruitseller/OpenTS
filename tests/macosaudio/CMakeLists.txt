add_executable(MacOSAudio
	macosaudio.cpp
	"${CMAKE_SOURCE_DIR}/code/platform/macos/dsound.cpp"
)
target_compile_features(MacOSAudio PRIVATE cxx_std_20)
target_compile_definitions(MacOSAudio PRIVATE OPENTS_MACOS)
target_compile_options(MacOSAudio PRIVATE -fms-extensions)
target_include_directories(MacOSAudio PRIVATE
	"${CMAKE_SOURCE_DIR}/code/platform/macos"
	"${CMAKE_SOURCE_DIR}/code"
)
target_link_libraries(MacOSAudio PRIVATE
	"-framework AudioToolbox"
	"-framework CoreAudio"
	"-framework CoreFoundation"
)

set_target_properties(MacOSAudio PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME macosaudio COMMAND MacOSAudio)
add_test(NAME macosaudio-playback COMMAND MacOSAudio --playback)
set_tests_properties(macosaudio-playback PROPERTIES SKIP_RETURN_CODE 77)
