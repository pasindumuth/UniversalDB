#include "MultiPaxosHandler.h"

namespace uni {
namespace paxos {

MultiPaxosHandler::MultiPaxosHandler(
  uni::paxos::PaxosLog& paxos_log,
  std::function<uni::paxos::SinglePaxosHandler(index_t)> instance_provider)
  : _paxos_log(paxos_log),
    _instance_provider(instance_provider) {}

void MultiPaxosHandler::propose(proto::paxos::PaxosLogEntry const& entry) {
  index_t index = _paxos_log.next_available_index(); // Look for an index that we can propose this new log entry to.
  auto& paxos_instance = get_instance(index);
  paxos_instance.propose(entry);
}

void MultiPaxosHandler::handle_incoming_message(
    uni::net::EndpointId const& endpoint_id, proto::paxos::PaxosMessage const& paxos_message) {
  auto& paxos_instance = get_instance(paxos_message.paxos_index());
  if (paxos_message.has_prepare()) {
    LOG(uni::logging::Level::TRACE2, "Prepare gotten.")
    paxos_instance.prepare(endpoint_id, paxos_message.prepare());
  } else if (paxos_message.has_promise()) {
    LOG(uni::logging::Level::TRACE2, "Promise gotten.")
    paxos_instance.promise(paxos_message.promise());
  } else if (paxos_message.has_accept()) {
    LOG(uni::logging::Level::TRACE2, "Accept gotten.")
    paxos_instance.accept(paxos_message.accept());
  } else if (paxos_message.has_learn()) {
    LOG(uni::logging::Level::TRACE2, "Learn gotten.")
    paxos_instance.learn(paxos_message.learn());
  }
}

uni::paxos::SinglePaxosHandler& MultiPaxosHandler::get_instance(index_t index) {
  if (_paxos_instances.find(index) == _paxos_instances.end()) {
    _paxos_instances.insert({index, _instance_provider(index)});
  }
  return _paxos_instances.find(index)->second;
}

} // paxos
} // uni
