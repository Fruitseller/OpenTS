add_executable(PEResource
	peresource.cpp
	"${CMAKE_SOURCE_DIR}/code/platform/macos/peresource.cpp"
)
target_compile_features(PEResource PRIVATE cxx_std_20)
target_compile_definitions(PEResource PRIVATE OPENTS_MACOS)
target_include_directories(PEResource PRIVATE "${CMAKE_SOURCE_DIR}/code/platform/macos")
target_compile_options(PEResource PRIVATE -fms-extensions)

set_target_properties(PEResource PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME peresource COMMAND PEResource)
