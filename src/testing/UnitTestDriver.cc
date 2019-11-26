#include "UnitTestDriver.h"

#include <logging/log.h>
#include <testing/slave/KVStoreTest.h>

namespace uni {
namespace testing {

using uni::testing::slave::KVStoreTest;

void UnitTestDriver::run_tests() {
  auto kvStoreTest = KVStoreTest();
  kvStoreTest.run_tests();
}

} // namespace testing
} // namespace uni
