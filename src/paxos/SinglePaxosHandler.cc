#include "SinglePaxosHandler.h"

#include <vector>

#include <assert/assert.h>
#include <utils/pbutil.h>

namespace uni {
namespace paxos {

PaxosProposerState::PaxosProposerState()
  : latest(0) {}

PaxosAcceptorState::PaxosAcceptorState()
  : accepted_state({0, 0, proto::paxos::PaxosLogEntry::default_instance()}) {}

PaxosLearnerState::PaxosLearnerState()
  : learned(false) {}

SinglePaxosHandler::SinglePaxosHandler(
  uni::constants::Constants const& constants,
  uni::net::Connections& connections,
  uni::paxos::PaxosLog& paxos_log,
  uni::random::Random& random,
  index_t paxos_log_index,
  std::vector<uni::net::EndpointId> config_endpoints,
  std::function<proto::message::MessageWrapper(proto::paxos::PaxosMessage*)> paxos_message_to_wrapper)
  : _constants(constants),
    _connections(connections),
    _paxos_log(paxos_log),
    _random(random),
    _paxos_log_index(paxos_log_index),
    _config_endpoints(config_endpoints),
    _paxos_message_to_wrapper(paxos_message_to_wrapper) {}

crnd_t SinglePaxosHandler::next_proposal_number() {
  return _proposer_state.latest + _random.rand_uniform(0, 999);
}

uint32_t SinglePaxosHandler::majority_threshold() {
  return std::floor(_constants.num_slave_servers / 2) + 1;
}

void SinglePaxosHandler::propose(const proto::paxos::PaxosLogEntry& entry) {
  auto proposal_number = next_proposal_number();
  _proposer_state.latest = proposal_number;
  _proposer_state.proposal.insert({proposal_number, entry});
  _proposer_state.prepare_state.insert({proposal_number, {}});
  auto paxos_message = new proto::paxos::PaxosMessage; // message_wrapper takes ownership and handles deleting this
  auto prepare_message = new proto::paxos::Prepare; // paxos_message takes ownership and handles deleting this
  prepare_message->set_rnd(proposal_number);
  paxos_message->set_allocated_prepare(prepare_message);
  paxos_message->set_paxos_index(_paxos_log_index);
  auto message_wrapper = _paxos_message_to_wrapper(paxos_message);
  _connections.broadcast(_config_endpoints, message_wrapper.SerializeAsString());
}

void SinglePaxosHandler::prepare(uni::net::EndpointId const& endpoint_id, proto::paxos::Prepare const& prepare_message) {
  auto latest_proposal_number = std::get<0>(_acceptor_state.accepted_state);
  auto new_proposal_number = prepare_message.rnd();
  if (new_proposal_number > latest_proposal_number) {
    // The incoming Proposal Number is higher than the current Proposal Number,
    // which means a Promise can be sent back.
    std::get<0>(_acceptor_state.accepted_state) = new_proposal_number;
    auto paxos_message = new proto::paxos::PaxosMessage; // message_wrapper takes ownership and handles deleting this
    auto promise = new proto::paxos::Promise; // paxos_message takes ownership and handles deleting this
    auto vval = new proto::paxos::PaxosLogEntry(std::get<2>(_acceptor_state.accepted_state)); // promise takes ownership and handles deleting this
    promise->set_allocated_vval(vval);
    promise->set_rnd(std::get<0>(_acceptor_state.accepted_state));
    promise->set_vrnd(std::get<1>(_acceptor_state.accepted_state));
    paxos_message->set_allocated_promise(promise);
    paxos_message->set_paxos_index(_paxos_log_index);
    auto message_wrapper = _paxos_message_to_wrapper(paxos_message);
    _connections.send(endpoint_id, message_wrapper.SerializeAsString());
  }
}

void SinglePaxosHandler::promise(proto::paxos::Promise const& promise_message) {
  auto rnd = promise_message.rnd();
  auto it = _proposer_state.prepare_state.find(rnd);
  UNIVERSAL_ASSERT_MESSAGE(it != _proposer_state.prepare_state.end(),
      "The prepare_state of a Proposal Number must have been initialized if it is to be received as a Promise")
  auto& promises = it->second;
  if (promises.size() < majority_threshold()) {
    // Haven't sent out an Accept message, and we need more Promises from other Universal Slaves.
    promises.push_back({promise_message.vrnd(), promise_message.vval()});
    if (promises.size() == majority_threshold()) {
      // Just got enough Promises to send out an Accept message to all Universal Slaves, so send it out.
      auto it = _proposer_state.proposal.find(rnd);
      UNIVERSAL_ASSERT_MESSAGE(it != _proposer_state.proposal.end(),
          "The Proposal for a Proposal Number must have been initialized if it is to be received as a Promise")
      auto proposal_value = it->second;
      // If a promising Slaves has already accepted a value, we ignore that original value we
      // were trying to propose at this node. Instead, take the value returned from the Slaves
      // with the highest Proposal Number, and use that.
      auto max_vrnd = 0;
      for (auto promise : promises) {
        auto vrnd = std::get<0>(promise);
        if (vrnd > 0) {
          // The Slave that send this Promise has already accepted a value.
          if (vrnd > max_vrnd) {
            proposal_value = std::get<1>(promise);
          }
        }
      }

      auto paxos_message = new proto::paxos::PaxosMessage; // message_wrapper takes ownership and handles deleting this
      auto accept_message = new proto::paxos::Accept; // paxos_message takes ownership and handles deleting this
      auto vval = new proto::paxos::PaxosLogEntry(proposal_value); // accept_message takes ownership and handles deleting this
      accept_message->set_vrnd(rnd);
      accept_message->set_allocated_vval(vval);
      paxos_message->set_allocated_accept(accept_message);
      paxos_message->set_paxos_index(_paxos_log_index);
      auto message_wrapper = _paxos_message_to_wrapper(paxos_message);
      _connections.broadcast(_config_endpoints, message_wrapper.SerializeAsString());
    }
  }
}

void SinglePaxosHandler::accept(proto::paxos::Accept const& accept_message) {
  auto cur_rnd = std::get<0>(_acceptor_state.accepted_state);
  auto new_rnd = accept_message.vrnd();
  if (new_rnd >= cur_rnd) {
    // This check is necessary even for Slaves that were contacted earlier
    // with a Prepare message from the Proposer, in case those Slaves received a
    // new Prepare or Accept message with an even higher Proposal Number.
    std::get<0>(_acceptor_state.accepted_state) = new_rnd;
    std::get<1>(_acceptor_state.accepted_state) = new_rnd;
    std::get<2>(_acceptor_state.accepted_state) = accept_message.vval();

    auto paxos_message = new proto::paxos::PaxosMessage; // message_wrapper takes ownership and handles deleting this
    auto learn_message = new proto::paxos::Learn; // paxos_message takes ownership and handles deleting this
    auto vval = new proto::paxos::PaxosLogEntry(accept_message.vval()); // learn_message takes ownership and handles deleting this
    learn_message->set_vrnd(new_rnd);
    learn_message->set_allocated_vval(vval);
    paxos_message->set_allocated_learn(learn_message);
    paxos_message->set_paxos_index(_paxos_log_index);
    auto message_wrapper = _paxos_message_to_wrapper(paxos_message);
    _connections.broadcast(_config_endpoints, message_wrapper.SerializeAsString());
  }
}

void SinglePaxosHandler::learn(proto::paxos::Learn const& learn_message) {
  if (_learner_state.learned) return; // If there is already a learned value, ignore this Learn message.
  auto rnd = learn_message.vrnd();
  auto it = _learner_state.learned_value.find(rnd);
  if (it == _learner_state.learned_value.end()) {
    // A Learn message with a Proposal Number of rnd has been received for the first time.
    _learner_state.learned_value.insert({rnd, {learn_message.vval(), 0}});
    it = _learner_state.learned_value.find(rnd);
  }
  auto& learned_count = std::get<1>(it->second);
  if (learned_count < majority_threshold()) {
    // The value of this Proposal has not been learned yet.
    learned_count++;
    if (learned_count == majority_threshold()) {
      // Just got enough Learn messages to consider the value of this Proposal to be learned.
      _learner_state.learned = true;
      // Update the PaxosLog with the newly learned value.
      _paxos_log.set_entry(_paxos_log_index, std::get<0>(it->second));
      LOG(uni::logging::Level::TRACE1, _paxos_log.debug_string())
    }
  }
}

} // namespace paxos
} // namespace uni
