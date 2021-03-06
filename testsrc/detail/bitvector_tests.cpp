#include <gatekit/detail/bitvector.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace gatekit {
namespace detail {

TEST(bitvector_tests, bitvector_alignment)
{
  bitvector under_test = bitvector::zeros();
  ASSERT_THAT(reinterpret_cast<uintptr_t>(&under_test) % 32, ::testing::Eq(0));
}

TEST(bitvector_tests, bitvector_map_alignment)
{
  bitvector_map under_test{1};
  ASSERT_THAT(reinterpret_cast<uintptr_t>(&under_test[0]) % 32, ::testing::Eq(0));
}

}
}
