add_executable(WinControls
	wincontrols.cpp
	"${CMAKE_SOURCE_DIR}/code/platform/macos/wincontrols.cpp"
	"${CMAKE_SOURCE_DIR}/code/platform/macos/peresource.cpp"
)
target_compile_features(WinControls PRIVATE cxx_std_20)
target_compile_definitions(WinControls PRIVATE OPENTS_MACOS)
target_include_directories(WinControls PRIVATE "${CMAKE_SOURCE_DIR}/code/platform/macos")
target_compile_options(WinControls PRIVATE -fms-extensions)

set_target_properties(WinControls PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME wincontrols COMMAND WinControls)
set_tests_properties(wincontrols PROPERTIES TIMEOUT 30)
