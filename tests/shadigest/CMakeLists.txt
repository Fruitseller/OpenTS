add_executable(SHADigest
	"${CMAKE_CURRENT_SOURCE_DIR}/shadigest.cpp"
	"${CMAKE_SOURCE_DIR}/code/sha.cpp"
)

target_compile_features(SHADigest PRIVATE cxx_std_20)
target_include_directories(SHADigest PRIVATE "${CMAKE_SOURCE_DIR}/code")
if(APPLE)
	target_include_directories(SHADigest PRIVATE "${CMAKE_SOURCE_DIR}/code/platform/macos")
endif()
if(WIN32)
	target_compile_definitions(SHADigest PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)
	target_compile_options(SHADigest PRIVATE
		$<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
		$<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
	)
endif()

set_target_properties(SHADigest PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME shadigest COMMAND SHADigest)
