#include "gtest/gtest.h"

#include <assert/UniversalException.h>
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
    _kvstore.write("k", "v3", 4);
    _kvstore.write("k1", "v4", 3);
   }

   uni::slave::KVStore _kvstore;
};

/**
 * Ensures that the values read for different keys at different timestamps
 * are correct.
 */ 
TEST_F(KVStoreTest, ReadTest) {
  EXPECT_EQ(_kvstore.read("k", 0), boost::none) << "Value read should be none.";
  EXPECT_EQ(_kvstore.read("k", 1).get(), "v1") << "Wrong reading value.";
  EXPECT_EQ(_kvstore.read("k", 3).get(), "v2") << "Wrong reading value.";
  EXPECT_EQ(_kvstore.read("k", 5).get(), "v3") << "Wrong reading value.";
  EXPECT_EQ(_kvstore.read("k1", 4).get(), "v4") << "Wrong reading value.";
}

/**
 * Ensures that a value cannot be written to a key with a timestamp before a previous
 * read or write of the key.
 */ 
TEST_F(KVStoreTest, WriteToThePastAfterRead) {
  // When reading a value at a given timestamp
  _kvstore.read("k1", 5).get();
  // And when trying to write a value to an earlier timestamp for the same key,
  // Then expect an exception to be thrown.
  ASSERT_THROW(_kvstore.write("k1", "v5", 4), uni::assert::UniversalException);
}

// /**
//  * Ensure that value can be written for the same key at different timestamps
//  * and the read values are as expected.
//  */
// TEST_F(KVStoreTest, WriteTest) {
//   EXPECT_EQ(_kvstore.read("k1", 5).get(), "v4") << "Wrong reading value.";
//   _kvstore.write("k1", "v5", 6);
//   EXPECT_EQ(_kvstore.read("k1", 5).get(), "v4") << "Wrong reading value.";
//   EXPECT_EQ(_kvstore.read("k1", 6).get(), "v5") << "Wrong reading value.";
// }

} // namespace slave
} // namespace unit
} // namespace testing
} // namespace uni
