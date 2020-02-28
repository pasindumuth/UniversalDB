#ifndef UNI_SLAVE_THREADPOOL_H
#define UNI_SLAVE_THREADPOOL_H

#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

namespace uni {
namespace slave {

struct ThreadPool {
  struct ThreadAndContext {
    boost::asio::io_context io_context;
    std::thread thread;

    ThreadAndContext();
  };

  std::vector<std::unique_ptr<ThreadAndContext>> io_contexts;

  ThreadPool();
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_THREADPOOL_H
