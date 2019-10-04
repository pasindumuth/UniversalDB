#include "MultiPaxosHandler.h"

#include <logging/log.h>
#include <proto/message.pb.h>

namespace uni {
namespace paxos {

using proto::message::MessageWrapper;
using proto::paxos::PaxosLogEntry;
using proto::paxos::PaxosMessage;
using uni::paxos::PaxosLog;
using uni::paxos::SinglePaxosHandler;

MultiPaxosHandler::MultiPaxosHandler(
    std::shared_ptr<PaxosLog> paxos_log,
    std::function<SinglePaxosHandler(index_t)> instance_provider)
    : _paxos_log(paxos_log),
      _instance_provider(instance_provider) {}

void MultiPaxosHandler::propose(PaxosLogEntry const& entry) {
  index_t index = _paxos_log->next_available_index(); // Look for an index that we can propose this new log entry to.
  auto& paxos_instance = get_instance(index);
  paxos_instance.propose(MessageWrapper(), entry);
}

void MultiPaxosHandler::handle_incoming_message(
    uni::net::endpoint_id const& endpoint_id, PaxosMessage const& paxos_message) {
  auto& paxos_instance = get_instance(paxos_message.paxos_index());
  if (paxos_message.has_prepare()) {
    LOG(uni::logging::Level::INFO, "prepare gotten.")
    paxos_instance.prepare(MessageWrapper(), endpoint_id, paxos_message.prepare());
  } else if (paxos_message.has_promise()) {
    LOG(uni::logging::Level::INFO, "promise gotten.")
    paxos_instance.promise(MessageWrapper(), paxos_message.promise());
  } else if (paxos_message.has_accept()) {
    LOG(uni::logging::Level::INFO, "accept gotten.")
    paxos_instance.accept(MessageWrapper(), paxos_message.accept());
  } else if (paxos_message.has_learn()) {
    LOG(uni::logging::Level::INFO, "learn gotten.")
    paxos_instance.learn(paxos_message.learn());
  }
}

SinglePaxosHandler& MultiPaxosHandler::get_instance(index_t index) {
  if (_paxos_instances.find(index) == _paxos_instances.end()) {
    _paxos_instances.insert({index, _instance_provider(index)});
  }
  return _paxos_instances.find(index)->second;
}

} // paxos
} // uni
