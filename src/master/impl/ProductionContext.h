#ifndef UNI_MASTER_PRODUCTIONCONTEXT_H
#define UNI_MASTER_PRODUCTIONCONTEXT_H

#include <functional>

#include <boost/asio.hpp>

#include <async/AsyncQueue.h>
#include <async/AsyncScheduler.h>
#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <constants/constants.h>
#include <common/common.h>
#include <master/DataMaster.h>
#include <net/Connections.h>
#include <random/impl/RandomImpl.h>

namespace uni {
namespace master {

struct ProductionContext {
  ProductionContext(
    boost::asio::io_context& background_io_context,
    uni::constants::Constants const& constants,
    uni::net::Connections& client_connections,
    uni::net::Connections& slave_connections,
    uni::net::Connections& connections,
    uni::async::AsyncScheduler& scheduler,
    std::vector<uni::net::EndpointId>& config_endpoints,
    std::vector<uni::net::EndpointId>& slave_endpoints);

  // Helper classes
  uni::random::RandomImpl _random;

  // Singletons
  uni::async::TimerAsyncSchedulerImpl _timer_scheduler;
  uni::master::DataMaster _master;

};

} // namespace master
} // namespace uni

#endif // UNI_MASTER_PRODUCTIONCONTEXT_H
