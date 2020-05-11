#ifndef UNI_SLAVE_PRODUCTIONCONTEXT_H
#define UNI_SLAVE_PRODUCTIONCONTEXT_H

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <async/AsyncScheduler.h>
#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <constants/constants.h>
#include <common/common.h>
#include <net/Connections.h>
#include <random/impl/RandomImpl.h>
#include <slave/LocalTransactionManager.h>

namespace uni {
namespace slave {

struct ProductionContext {
  struct ThreadAndContext {
    boost::asio::io_context _io_context;
    std::thread _thread;

    ThreadAndContext();
  };

  ProductionContext(
    boost::asio::io_context& background_io_context,
    uni::constants::Constants const& constants,
    uni::net::Connections& client_connections,
    uni::net::Connections& master_connections,
    uni::net::Connections& slave_connections,
    uni::async::AsyncScheduler& scheduler,
    std::vector<uni::net::EndpointId> const& config_endpoints,
    std::string ip_string);

  // Singletons
  uni::random::RandomImpl _random;
  uni::async::TimerAsyncSchedulerImpl _timer_scheduler;
  uni::slave::LocalTransactionManager _transaction_manager;

 private:
  std::vector<unsigned> _participants_per_thread;
  std::vector<std::unique_ptr<ThreadAndContext>> _io_contexts;
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_PRODUCTIONCONTEXT_H
