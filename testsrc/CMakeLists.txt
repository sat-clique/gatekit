if (GATEKIT_ENABLE_TESTS)
  add_executable(gatekit-tests
    detail/bitvector_partition_tests.cpp
    detail/bitvector_prop_tests.cpp
    detail/bitvector_rand_tests.cpp
    detail/bitvector_tests.cpp
    detail/blocked_set_tests.cpp
    detail/collections_tests.cpp
    detail/occurrence_list_tests.cpp
    detail/scanner_gate_tests.cpp
    detail/utils_tests.cpp

    helpers/gate_factory.cpp

    random_simulation_tests.cpp
    scanner_tests.cpp
  )

  target_link_libraries(gatekit-tests PRIVATE gatekit gtest gmock gmock_main)

  if (GATEKIT_GNULIKE_COMPILER)
    target_compile_options(gatekit-tests PRIVATE -Wall -Wextra -pedantic)
  endif()
endif()

