if (GATEKIT_ENABLE_TESTS)
  add_executable(gatekit-tests
    detail/blocked_set_tests.cpp
    detail/occurrence_list_tests.cpp
  )

  target_link_libraries(gatekit-tests PRIVATE gatekit gtest gmock gmock_main)

  if (GATEKIT_GNULIKE_COMPILER)
    target_compile_options(gatekit-tests PRIVATE -Wall -Wextra -pedantic)
  endif()
endif()

