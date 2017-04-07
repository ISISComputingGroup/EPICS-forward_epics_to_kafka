set(REQUIRE_GTEST FALSE CACHE BOOL "Require Google Test")
if (REQUIRE_GTEST)
	find_path(path_googletest_repository NAMES "googletest/include/gtest/gtest.h" HINTS "../googletest")
	if (path_googletest_repository)
		set(path_include_gtest "${path_googletest_repository}/googletest/include")
		message(STATUS "Google Test repository at ${path_googletest_repository}")
		set(have_gtest 1)
		add_subdirectory(${path_googletest_repository} googletest)
	else()
		message(FATAL_ERROR "Google Test required but not found")
	endif()
endif()

if (have_gtest)
	add_library(gtest_static STATIC IMPORTED)
	set_property(TARGET gtest_static PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/googletest/googlemock/gtest/libgtest.a")
endif()

function(add_gtest_to_target tgt)
	if (have_gtest)
		add_dependencies(${tgt} gtest)
		target_compile_definitions(${tgt} PRIVATE HAVE_GTEST=1)
		target_include_directories(${tgt} PRIVATE ${path_include_gtest})
		target_link_libraries(${tgt} gtest_static)
	endif()
endfunction()
