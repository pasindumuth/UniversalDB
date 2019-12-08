#include "gtest/gtest.h"

#include <slave/KVStore.h>

namespace uni {
namespace testing {
namespace unit {
namespace slave {

class KVStoreTest
    : public ::testing::Test {
  protected:
   void SetUp() override {
    _kvstore.write("k", "v1", 0);
    _kvstore.write("k", "v2", 2);
    _kvstore.write("k", "v4", 4);
    _kvstore.write("k1", "v1", 3);
   }

   uni::slave::KVStore _kvstore;
};

TEST_F(KVStoreTest, ReadTest) {
  EXPECT_EQ(_kvstore.read("k", 0), boost::none) << "Value read should be none.";
  EXPECT_EQ(_kvstore.read("k", 1).get(), "v1") << "Wrong reading value.";
  EXPECT_EQ(_kvstore.read("k", 3).get(), "v2") << "Wrong reading value.";
  EXPECT_EQ(_kvstore.read("k", 5).get(), "v4") << "Wrong reading value.";
  EXPECT_EQ(_kvstore.read("k1", 4).get(), "v1") << "Wrong reading value.";
}

} // namespace slave
} // namespace unit
} // namespace testing
} // namespace uni
