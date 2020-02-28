#include "ThreadPool.h"

namespace uni {
namespace slave {

ThreadPool::ThreadAndContext::ThreadAndContext()
  : io_context(),
    thread([this] {
      auto work_guard = boost::asio::make_work_guard(io_context);
      io_context.run();
    }) {}

ThreadPool::ThreadPool() {
  // We subtract 2 from the number of supported threads to account for the
  // LTM thread and background thread are started up manually.
  auto num_supported_threads = std::thread::hardware_concurrency() - 2; 
  for (auto i = 0; i < num_supported_threads; i++) {
    io_contexts.push_back(std::make_unique<ThreadAndContext>());
  }
}

} // namespace slave
} // namespace uni
