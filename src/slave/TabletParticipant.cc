#include "TabletParticipant.h"

namespace uni {
namespace slave {

TabletParticipant::TabletParticipant(
  uni::constants::Constants const& constants,
  uni::net::ConnectionsOut& connections_out,
  uni::net::ConnectionsIn& client_connections_in,
  uni::async::TimerAsyncScheduler& timer_scheduler,
  uni::slave::FailureDetector& failure_detector)
  : io_context(),
    scheduler(io_context),
    paxos_log(),
    multipaxos_handler(
      paxos_log,
      [this, &constants, &connections_out](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(constants, connections_out, paxos_log, index);
      }),
    proposer_queue(timer_scheduler),
    kvstore(),
    client_request_handler(
      multipaxos_handler,
      paxos_log,
      proposer_queue,
      kvstore,
      [this, &client_connections_in](
        uni::net::endpoint_id endpoint_id,
        proto::client::ClientResponse* client_response
      ) {
        auto client_message = new proto::client::ClientMessage();
        client_message->set_allocated_response(client_response);
        auto message_wrapper = proto::message::MessageWrapper();
        message_wrapper.set_allocated_client_message(client_message);
        auto channel = client_connections_in.get_channel(endpoint_id);
        if (channel) {
          channel.get()->queue_send(message_wrapper.SerializeAsString());
        } else {
          LOG(uni::logging::Level::WARN, "Client Channel to reply to no longer exists.");
        }
      }),
    heartbeat_tracker(),
    log_syncer(
      constants,
      connections_out,
      timer_scheduler,
      paxos_log,
      failure_detector),
    incoming_message_handler(
      client_request_handler,
      heartbeat_tracker,
      log_syncer,
      multipaxos_handler),
    thread([this](){
      auto work_guard = boost::asio::make_work_guard(io_context);
      io_context.run();
    }) {
  scheduler.set_callback([this](uni::net::IncomingMessage message){
    incoming_message_handler.handle(message);
  });
}

} // namespace slave
} // namespace uni
