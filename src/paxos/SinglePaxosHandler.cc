#include "SinglePaxosHandler.h"

#include <cmath>
#include <vector>

#include <assert/assert.h>
#include <logging/log.h>

namespace uni {
namespace paxos {

using proto::message::MessageWrapper;
using proto::paxos::Accept;
using proto::paxos::Learn;
using proto::paxos::PaxosLogEntry;
using proto::paxos::PaxosMessage;
using proto::paxos::Prepare;
using proto::paxos::Promise;
using uni::constants::Constants;
using uni::net::ConnectionsOut;
using uni::paxos::PaxosLog;

SinglePaxosHandler::SinglePaxosHandler(
    Constants const& constants,
    ConnectionsOut& connections_out,
    PaxosLog& paxos_log,
    index_t paxos_log_index)
      : _constants(constants),
        _connections_out(connections_out),
        _paxos_log(paxos_log),
        _paxos_log_index(paxos_log_index) {}

crnd_t SinglePaxosHandler::next_proposal_number() {
  return _proposer_state.latest + 10; // TODO: pick this randomly.
}

unsigned SinglePaxosHandler::majority_threshold() {
  return std::floor(_constants.num_slave_servers / 2) + 1;
}

void SinglePaxosHandler::propose(
    MessageWrapper message_wrapper, const PaxosLogEntry& entry) {
  auto proposal_number = next_proposal_number();
  _proposer_state.latest = proposal_number;
  _proposer_state.proposal.insert({proposal_number, entry});
  _proposer_state.prepare_state.insert({proposal_number, {}});
  auto paxos_message = new PaxosMessage; // message_wrapper takes ownership and handles deleting this
  auto prepare_message = new Prepare; // paxos_message takes ownership and handles deleting this
  prepare_message->set_rnd(proposal_number);
  paxos_message->set_allocated_prepare(prepare_message);
  paxos_message->set_paxos_index(_paxos_log_index);
  message_wrapper.set_allocated_paxos_message(paxos_message);
  _connections_out.broadcast(message_wrapper.SerializeAsString());
}

void SinglePaxosHandler::prepare(
    MessageWrapper message_wrapper, uni::net::endpoint_id const& endpoint_id, Prepare const& prepare_message) {
  auto latest_proposal_number = std::get<0>(_acceptor_state.accepted_state);
  auto new_proposal_number = prepare_message.rnd();
  if (new_proposal_number > latest_proposal_number) {
    // The incoming proposal number is higher than the current proposal number,
    // which means a promise can be sent back.
    std::get<0>(_acceptor_state.accepted_state) = new_proposal_number;
    auto paxos_message = new PaxosMessage; // message_wrapper takes ownership and handles deleting this
    auto promise = new Promise; // paxos_message takes ownership and handles deleting this
    auto vval = new PaxosLogEntry(std::get<2>(_acceptor_state.accepted_state)); // promise takes ownership and handles deleting this
    promise->set_allocated_vval(vval);
    promise->set_rnd(std::get<0>(_acceptor_state.accepted_state));
    promise->set_vrnd(std::get<1>(_acceptor_state.accepted_state));
    paxos_message->set_allocated_promise(promise);
    paxos_message->set_paxos_index(_paxos_log_index);
    message_wrapper.set_allocated_paxos_message(paxos_message);
    _connections_out.send(endpoint_id, message_wrapper.SerializeAsString());
  }
}

void SinglePaxosHandler::promise(
    MessageWrapper message_wrapper, Promise const& promise_message) {
  auto rnd = promise_message.rnd();
  auto it = _proposer_state.prepare_state.find(rnd);
  UNIVERSAL_ASSERT_MESSAGE(it != _proposer_state.prepare_state.end(),
      "The prepare_state of a proposal number must have been initialized if it is to be recieved as a promise")
  auto& promises = it->second;
  if (promises.size() < majority_threshold()) {
    // Haven't sent out an Accept message, and we need more Promises from other Universal Slaves.
    promises.push_back({promise_message.vrnd(), promise_message.vval()});
    if (promises.size() == majority_threshold()) {
      // Just got enough Promises to send out an Accept message to all Universal Slaves, so send it out.
      auto it = _proposer_state.proposal.find(rnd);
      UNIVERSAL_ASSERT_MESSAGE(it != _proposer_state.proposal.end(),
          "The proposal for a proposal number must have been initialized if it is to be recieved as a promise")
      auto proposal_value = it->second;
      // If a promising Slaves has already accepted a value, we ignore that original value we
      // were trying to propose at this node. Instead, take the value returned from the Slaves
      // with the highest proposal number, and use that. This is part of the Paxos algorithm.
      unsigned max_vrnd = 0;
      for (auto promise : promises) {
        unsigned vrnd = std::get<0>(promise);
        if (vrnd > 0) {
          // The Slave that send this promise already accepted a value.
          if (vrnd > max_vrnd) {
            proposal_value = std::get<1>(promise);
          }
        }
      }

      auto paxos_message = new PaxosMessage; // message_wrapper takes ownership and handles deleting this
      auto accept_message = new Accept; // paxos_message takes ownership and handles deleting this
      auto vval = new PaxosLogEntry(proposal_value); // accept_message takes ownership and handles deleting this
      accept_message->set_vrnd(rnd);
      accept_message->set_allocated_vval(vval);
      paxos_message->set_allocated_accept(accept_message);
      paxos_message->set_paxos_index(_paxos_log_index);
      message_wrapper.set_allocated_paxos_message(paxos_message);
      _connections_out.broadcast(message_wrapper.SerializeAsString());
    }
  }
}

void SinglePaxosHandler::accept(
    MessageWrapper message_wrapper, Accept const& accept_message) {
  auto cur_rnd = std::get<0>(_acceptor_state.accepted_state);
  auto new_rnd = accept_message.vrnd();
  if (new_rnd >= cur_rnd) {
    // This check is necessary even for Slaves that were contacted earlier
    // with a Prepare message from the proposer, in case those Slaves recieved a
    // new Prepare or Accept message with an even higher proposal number.
    std::get<0>(_acceptor_state.accepted_state) = new_rnd;
    std::get<1>(_acceptor_state.accepted_state) = new_rnd;
    std::get<2>(_acceptor_state.accepted_state) = accept_message.vval();

    auto paxos_message = new PaxosMessage; // message_wrapper takes ownership and handles deleting this
    auto learn_message = new Learn; // paxos_message takes ownership and handles deleting this
    auto vval = new PaxosLogEntry(accept_message.vval()); // learn_message takes ownership and handles deleting this
    learn_message->set_vrnd(new_rnd);
    learn_message->set_allocated_vval(vval);
    paxos_message->set_allocated_learn(learn_message);
    paxos_message->set_paxos_index(_paxos_log_index);
    message_wrapper.set_allocated_paxos_message(paxos_message);
    _connections_out.broadcast(message_wrapper.SerializeAsString());
  }
}

void SinglePaxosHandler::learn(Learn const& learn_message) {
  if (_learner_state.learned) return; // If there is already a learned value, ignore this Learn message.
  auto rnd = learn_message.vrnd();
  auto it = _learner_state.learned_value.find(rnd);
  if (it == _learner_state.learned_value.end()) {
    // A learn message with a proposal number of rnd has been recieved for the first time.
    _learner_state.learned_value.insert({rnd, {learn_message.vval(), 0}});
    it = _learner_state.learned_value.find(rnd);
  }
  auto& learned_count = std::get<1>(it->second);
  if (learned_count < majority_threshold()) {
    // The value of this proposal has not been learned yet.
    learned_count++;
    if (learned_count == majority_threshold()) {
      // Just got enough learn messages to consider the value of this proposal to be learned.
      _learner_state.learned = true;
      // Update the Paxos Log with the newly learned value.
      _paxos_log.set_entry(_paxos_log_index, std::get<0>(it->second));
    }
  }
}

} // paxos
} // uni

