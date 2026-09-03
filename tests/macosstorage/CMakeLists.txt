add_executable(MacOSStorage
	macosstorage.cpp
	"${CMAKE_SOURCE_DIR}/code/platform/macos/storage.cpp"
)
target_compile_features(MacOSStorage PRIVATE cxx_std_20)
target_compile_definitions(MacOSStorage PRIVATE OPENTS_MACOS)
target_include_directories(MacOSStorage PRIVATE "${CMAKE_SOURCE_DIR}/code/platform/macos")
target_compile_options(MacOSStorage PRIVATE -fms-extensions)

set_target_properties(MacOSStorage PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME macosstorage COMMAND MacOSStorage)
