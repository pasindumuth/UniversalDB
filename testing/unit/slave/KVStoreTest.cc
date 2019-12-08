#include "KVStoreTest.h"

#include <boost/optional/optional_io.hpp>

#include <assert/assert.h>
#include <logging/log.h>
#include <slave/KVStore.h>

namespace uni {
namespace testing {
namespace slave {

using uni::slave::KVStore;

void KVStoreTest::run_tests() {
  test1();
}

void KVStoreTest::test1() {
  LOG(uni::logging::Level::INFO, "Running KVStoreTest")
  auto kvstore = KVStore();
  kvstore.write("k", "v1", 0);
  kvstore.write("k", "v2", 2);
  kvstore.write("k", "v4", 4);
  kvstore.write("k1", "v1", 3);
  UNIVERSAL_ASSERT_MESSAGE(kvstore.read("k", 0) == boost::none, "Value read should be none.")
  UNIVERSAL_ASSERT_MESSAGE(kvstore.read("k", 1).get() == "v1", "Wrong reading value.")
  UNIVERSAL_ASSERT_MESSAGE(kvstore.read("k", 3).get() == "v2", "Wrong reading value.")
  UNIVERSAL_ASSERT_MESSAGE(kvstore.read("k", 5).get() == "v4", "Wrong reading value.")
  UNIVERSAL_ASSERT_MESSAGE(kvstore.read("k1", 4).get() == "v1", "Wrong reading value.")
  LOG(uni::logging::Level::INFO, "KVStoreTest [TEST PASSED]")
}

} // namespace slave
} // namespace testing
} // namespace uni
