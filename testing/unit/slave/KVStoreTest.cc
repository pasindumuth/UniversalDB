#include "gtest/gtest.h"

#include <boost/optional.hpp>

#include <assert/UniversalException.h>
#include <slave/KVStore.h>

namespace uni {
namespace testing {
namespace unit {
namespace slave {

class KVStoreTest
    : public ::testing::Test {
  protected:
   uni::slave::KVStore _kvstore;
};

/////////////// TESTS with only one key k1 ///////////////

/**
 * Tests if it's possible to do one write.
 */
TEST_F(KVStoreTest, FirstWriteTest) {
  EXPECT_EQ(_kvstore.read("k1", 1), boost::none) << "Wrong value read";
  _kvstore.write("k1", "v1", 1);
  EXPECT_EQ(_kvstore.read("k1", 1).get(), "v1") << "Wrong value read";
}

/**
 * Tests that after a value is written for a key, that another value
 * cannot be writtein to the same key with an (non-strictly) earlier timestamp.
 */
TEST_F(KVStoreTest, WriteToThePastAfterWrite) {
  _kvstore.write("k1", "v1", 10);
  ASSERT_THROW(_kvstore.write("k1", "v2", 5), uni::assert::UniversalException)
    << "Shouldn't be possible to write to a timestamp prior to the last write";
  ASSERT_THROW(_kvstore.write("k1", "v2", 10), uni::assert::UniversalException)
    << "Shouldn't be possible to write to a timestamp prior to the last write";
}

/**
 * Tests that it's possible to write to the same key with increasing timestamps,
 * and that the values read at different timestamps are correct.
 */
TEST_F(KVStoreTest, WriteWithIncreasingTimestamp) {
  _kvstore.write("k1", "v1", 1);
  EXPECT_NO_THROW(_kvstore.write("k1", "v2", 2))
    << "Should be possible to write at this time.";;
}

/**
 * Tests whether the values read are the correct when a key has multiple
 * versions at multiple timestamps.
 */
TEST_F(KVStoreTest, WriteWithIncreasingTimestampCorrect) {
  _kvstore.write("k1", "v1", 1);
  _kvstore.write("k1", "v2", 2);
  _kvstore.write("k1", "v3", 4);
  EXPECT_EQ(_kvstore.read("k1", 0), boost::none) << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k1", 1).get(), "v1") << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k1", 2).get(), "v2") << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k1", 3).get(), "v2") << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k1", 4).get(), "v3") << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k1", 5).get(), "v3") << "Wrong value read";
}

/**
 * Tests that after a value is written for a key, and it's read with a higher timestamp,
 * that another value cannot be writtein to the same key with an earlier timestamp.
 */
TEST_F(KVStoreTest, WriteToThePastAfterRead) {
  _kvstore.write("k1", "v1", 10);
  _kvstore.read("k1", 15);
  ASSERT_THROW(_kvstore.write("k1", "v2", 12), uni::assert::UniversalException)
    << "Shouldn't be possible to write to a timestamp prior to the last read";
  ASSERT_THROW(_kvstore.write("k1", "v2", 15), uni::assert::UniversalException)
    << "Shouldn't be possible to write to a timestamp prior to the last read";
}

/**
 * Makes sure a value can be written after a read.
 */
TEST_F(KVStoreTest, WriteAfterRead) {
  _kvstore.write("k1", "v1", 1);
  _kvstore.read("k1", 1);
  _kvstore.write("k1", "v2", 2);
  EXPECT_EQ(_kvstore.read("k1", 2).get(), "v2") << "Wrong value read";
}

/////////////// TESTS with more than one key ///////////////

/**
 * Tests that it's possible to insert multiple keys and retrieve the right value.
 */
TEST_F(KVStoreTest, MultipleKeyWrites) {
  _kvstore.write("k1", "v1", 1);
  _kvstore.write("k2", "v2", 1);
  _kvstore.write("k2", "v3", 2);
  EXPECT_EQ(_kvstore.read("k1", 0), boost::none) << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k1", 1).get(), "v1") << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k1", 2).get(), "v1") << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k2", 0), boost::none) << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k2", 1).get(), "v2") << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k2", 2).get(), "v3") << "Wrong value read";
  EXPECT_EQ(_kvstore.read("k2", 3).get(), "v3") << "Wrong value read";
}

/**
 * Tests that it's possible to write values into the past for different
 * keys.
 */
TEST_F(KVStoreTest, WriteToPastOnADifferentKey) {
  _kvstore.write("k1", "v1", 2);
  EXPECT_NO_THROW(_kvstore.write("k2", "v2", 1));
}

} // namespace slave
} // namespace unit
} // namespace testing
} // namespace uni