cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

project(gatekit VERSION 0.1.0)

if (NOT CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
  # This has been added via add_subdirectory()

  add_library(gatekit::gatekit INTERFACE IMPORTED)
  target_include_directories(gatekit::gatekit INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
else()
  # Building in standalone mode, ie. this is not included via add_subdirectory()
  # in another project

  option(GATEKIT_ENABLE_TESTS "Enable testing" OFF)
  option(GATEKIT_TEST_ENABLE_SANITIZERS "Enable sanitizers for tests" OFF)
  option(GATEKIT_BUILD_DOCS "Build Doxygen documentation" OFF)

  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_EXTENSIONS OFF)

  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(GATEKIT_GNULIKE_COMPILER TRUE)
  endif()

  if (GATEKIT_TEST_ENABLE_SANITIZERS)
    if (NOT GATEKIT_GNULIKE_COMPILER)
      message(FATAL_ERROR "Setting sanitizer options is not supported for this compiler")
    endif()

    # Adding sanitizer flags globally to keep gtest and test compile flags the same
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
  endif()

  add_library(gatekit INTERFACE)
  target_include_directories(gatekit INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

  install(DIRECTORY include/gatekit DESTINATION include)
  install(TARGETS gatekit EXPORT gatekit INCLUDES DESTINATION include)
  install(EXPORT gatekit NAMESPACE gatekit:: DESTINATION lib/cmake/gatekit FILE "gatekitConfig.cmake")

  add_subdirectory(doc)
  add_subdirectory(testdeps)
  add_subdirectory(testsrc)
endif()

