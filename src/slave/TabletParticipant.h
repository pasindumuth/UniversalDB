#ifndef UNI_SLAVE_TABLETPARTICIPANT_H
#define UNI_SLAVE_TABLETPARTICIPANT_H

#include <functional>
#include <memory>
#include <thread>

#include <boost/asio.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <async/AsyncScheduler.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/Connections.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <slave/ClientRequestHandler.h>
#include <slave/IncomingMessageHandler.h>
#include <slave/KVStore.h>
#include <server/LogSyncer.h>
#include <async/AsyncQueue.h>
#include <slave/TabletId.h>

namespace uni {
namespace slave {

struct TabletParticipant {
  std::unique_ptr<uni::async::AsyncScheduler> scheduler;
  uni::slave::TabletId tablet_id;
  uni::paxos::PaxosLog paxos_log;
  uni::paxos::MultiPaxosHandler multipaxos_handler;
  uni::async::AsyncQueue proposer_queue;
  uni::slave::KVStore kvstore;
  uni::slave::ClientRequestHandler client_request_handler;
  uni::server::LogSyncer log_syncer;
  uni::slave::IncomingMessageHandler incoming_message_handler;

  TabletParticipant(
    std::function<std::unique_ptr<uni::async::AsyncScheduler>()> scheduler_provider,
    uni::constants::Constants const& constants,
    uni::net::Connections& connections,
    uni::net::Connections& client_connections,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::server::FailureDetector& failure_detector,
    uni::slave::TabletId& tid);
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_TABLETPARTICIPANT_H
