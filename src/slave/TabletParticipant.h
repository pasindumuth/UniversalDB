#ifndef UNI_SLAVE_TABLETPARTICIPANT_H
#define UNI_SLAVE_TABLETPARTICIPANT_H

#include <thread>

#include <boost/asio.hpp>

#include <async/impl/AsyncSchedulerImpl.h>
#include <async/impl/TimerAsyncSchedulerImpl.h>
#include <common/common.h>
#include <constants/constants.h>
#include <net/ConnectionsIn.h>
#include <net/endpoint_id.h>
#include <net/IncomingMessage.h>
#include <net/impl/ChannelImpl.h>
#include <proto/client.pb.h>
#include <proto/master.pb.h>
#include <proto/message.pb.h>
#include <proto/tablet.pb.h>
#include <paxos/MultiPaxosHandler.h>
#include <paxos/PaxosLog.h>
#include <paxos/PaxosTypes.h>
#include <paxos/SinglePaxosHandler.h>
#include <slave/ClientConnectionHandler.h>
#include <slave/ClientRequestHandler.h>
#include <slave/IncomingMessageHandler.h>
#include <slave/KVStore.h>
#include <slave/LogSyncer.h>
#include <slave/ProposerQueue.h>
#include <slave/ServerConnectionHandler.h>
#include <slave/TabletId.h>

namespace uni {
namespace slave {

struct TabletParticipant {
  uni::slave::TabletId tablet_id;
  boost::asio::io_context io_context;
  uni::async::AsyncSchedulerImpl scheduler;
  uni::paxos::PaxosLog paxos_log;
  uni::paxos::MultiPaxosHandler multipaxos_handler;
  uni::slave::ProposerQueue proposer_queue;
  uni::slave::KVStore kvstore;
  uni::slave::ClientRequestHandler client_request_handler;
  uni::slave::HeartbeatTracker heartbeat_tracker;
  uni::slave::LogSyncer log_syncer;
  uni::slave::IncomingMessageHandler incoming_message_handler;
  std::thread thread;

  TabletParticipant(
    uni::constants::Constants const& constants,
    uni::net::ConnectionsOut& connections_out,
    uni::net::ConnectionsIn& client_connections_in,
    uni::async::TimerAsyncScheduler& timer_scheduler,
    uni::slave::FailureDetector& failure_detector,
    uni::slave::TabletId& tid);
};

} // namespace slave
} // namespace uni

#endif // UNI_SLAVE_TABLETPARTICIPANT_H
