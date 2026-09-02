if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
	add_executable(VqaDecode
		"${CMAKE_CURRENT_SOURCE_DIR}/vqadecode.cpp"
		"${CMAKE_SOURCE_DIR}/code/vqalib/unvq.cpp"
	)

	target_compile_features(VqaDecode PRIVATE cxx_std_20)
	target_compile_definitions(VqaDecode PRIVATE WIN32 _WINDOWS _MBCS NOMINMAX)
	target_compile_options(VqaDecode PRIVATE
		$<$<CONFIG:Debug>:/MTd /EHsc /Zc:__cplusplus>
		$<$<CONFIG:Release>:/MT /EHsc /Zc:__cplusplus>
	)

	set_target_properties(VqaDecode PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	)

	add_test(NAME vqadecode COMMAND VqaDecode)
endif()
