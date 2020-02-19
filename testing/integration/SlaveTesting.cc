#include "SlaveTesting.h"

#include <utils/pbutil.h>

namespace uni {
namespace testing {
namespace integration {

SlaveTesting::SlaveTesting(
  uni::slave::TabletId& tid,
  uni::constants::Constants const& constants)
  : tablet_id(tid),
    scheduler(),
    clock(),
    timer_scheduler(clock),
    connections_out(constants),
    paxos_log(),
    multipaxos_handler(
      paxos_log,
      [this, constants](uni::paxos::index_t index) {
        return uni::paxos::SinglePaxosHandler(
          constants,
          connections_out,
          paxos_log,
          index,
          [this](proto::paxos::PaxosMessage* paxos_message) {
            auto message_wrapper = proto::message::MessageWrapper();
            auto tablet_message = new proto::tablet::TabletMessage;
            tablet_message->set_allocated_database_id(uni::utils::pb::string(tablet_id.database_id));
            tablet_message->set_allocated_table_id(uni::utils::pb::string(tablet_id.table_id));
            tablet_message->set_allocated_paxos_message(paxos_message);
            message_wrapper.set_allocated_tablet_message(tablet_message);
            return message_wrapper;
          });
      }),
    proposer_queue(timer_scheduler),
    kvstore(),
    client_request_handler(
      multipaxos_handler,
      paxos_log,
      proposer_queue,
      kvstore,
      [](uni::net::endpoint_id, proto::client::ClientResponse*){}),
    heartbeat_tracker(),
    failure_detector(
      heartbeat_tracker,
      connections_out,
      timer_scheduler),
    log_syncer(
      constants,
      connections_out,
      timer_scheduler,
      paxos_log,
      failure_detector,
      [this](proto::sync::SyncMessage* sync_message){
        auto message_wrapper = proto::message::MessageWrapper();
        auto tablet_message = new proto::tablet::TabletMessage;
        tablet_message->set_allocated_database_id(uni::utils::pb::string(tablet_id.database_id));
        tablet_message->set_allocated_table_id(uni::utils::pb::string(tablet_id.table_id));
        tablet_message->set_allocated_sync_message(sync_message);
        message_wrapper.set_allocated_tablet_message(tablet_message);
        return message_wrapper;
      }),
    incoming_message_handler(
      client_request_handler,
      heartbeat_tracker,
      log_syncer,
      multipaxos_handler) {
  scheduler.set_callback([this](uni::net::IncomingMessage message) {
    incoming_message_handler.handle(message);
  });
  paxos_log.add_callback(kvstore.get_paxos_callback());
}

} // integration
} // testing
} // uni
