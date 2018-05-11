cmake_minimum_required(VERSION 2.8.11)
project(forward-epics-to-kafka)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")

if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(NO_OUTPUT_DIRS)
endif()

# for static build etc.
set(FETK_EXTRA_LIBRARIES "" CACHE STRING "Extra Libraries")
set(FETK_BUILD_TYPE "SHARED" CACHE STRING "Build Type")

if (${FETK_BUILD_TYPE} STREQUAL "STATIC")
  foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
  add_definitions(-DLIBRDKAFKA_STATICLIB -DEPICS_DLL_NO)
endif()


# Yes, there exists also FindGTest.
# Yes, we do want to use this one instead:
find_package(Googletest)

add_subdirectory(src)

option(RUN_DOXYGEN "Run doxygen" TRUE)
if (RUN_DOXYGEN)
configure_file(Doxygen.conf dg.conf)
add_custom_target(docs COMMAND doxygen dg.conf DEPENDS forward-epics-to-kafka)
endif()
