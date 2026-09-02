if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
	add_executable(LCWRoundTrip
		"${CMAKE_CURRENT_SOURCE_DIR}/lcwroundtrip.cpp"
		"${CMAKE_SOURCE_DIR}/code/lcw.cpp"
	)

	target_compile_features(LCWRoundTrip PRIVATE cxx_std_20)
	target_include_directories(LCWRoundTrip PRIVATE "${CMAKE_SOURCE_DIR}/code")
	target_compile_definitions(LCWRoundTrip PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)
	target_compile_options(LCWRoundTrip PRIVATE
		$<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
		$<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
	)

	set_target_properties(LCWRoundTrip PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	)

	add_test(NAME lcwroundtrip COMMAND LCWRoundTrip)
endif()
