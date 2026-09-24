add_executable(MacOSWindow
	macoswindow.mm
	"${CMAKE_SOURCE_DIR}/code/platform/macos/peresource.cpp"
	"${CMAKE_SOURCE_DIR}/code/platform/macos/macoswindow.mm"
)
target_compile_features(MacOSWindow PRIVATE cxx_std_20)
target_compile_definitions(MacOSWindow PRIVATE OPENTS_MACOS OPENTS_MACOS_WINDOW_TEST)
target_compile_options(MacOSWindow PRIVATE -fms-extensions)
target_include_directories(MacOSWindow PRIVATE
	"${CMAKE_SOURCE_DIR}/code/platform/macos"
	"${CMAKE_SOURCE_DIR}/code"
)
target_link_libraries(MacOSWindow PRIVATE
	"-framework AppKit"
	"-framework CoreGraphics"
)

set_target_properties(MacOSWindow PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME macoswindow-focus COMMAND MacOSWindow focus)
add_test(NAME macoswindow-fullscreen-level COMMAND MacOSWindow fullscreen-level)
add_test(NAME macoswindow-fullscreen-presentation COMMAND MacOSWindow fullscreen-presentation)
add_test(NAME macoswindow-quit COMMAND MacOSWindow quit)
add_test(NAME macoswindow-command-q COMMAND MacOSWindow command-q)

add_test(NAME macoswindow-text-input COMMAND MacOSWindow text-input)
