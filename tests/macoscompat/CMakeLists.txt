add_executable(MacOSCompat
	macoscompat.cpp
)
target_compile_features(MacOSCompat PRIVATE cxx_std_20)
target_compile_definitions(MacOSCompat PRIVATE OPENTS_MACOS OPENTS_MACOS_COMPAT_TEST)
target_compile_options(MacOSCompat PRIVATE -fms-extensions)
target_include_directories(MacOSCompat PRIVATE
	"${CMAKE_SOURCE_DIR}/code/platform/macos"
	"${CMAKE_SOURCE_DIR}/code"
)

set_target_properties(MacOSCompat PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME macoscompat COMMAND MacOSCompat)
