#ifndef UNI_PAXOS_PAXOSLOG_H
#define UNI_PAXOS_PAXOSLOG_H

#include <unordered_map>
#include <boost/optional.hpp>

#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

// The log of values that an instance of the MultiPaxos algorithms tries to add
// values to http://localhost:3000/projects/universaldb/multipaxos.
class PaxosLog {
 public:
  PaxosLog();

  // Returns the entry at the index if it exists, otherwise returns an empty optional
  boost::optional<proto::paxos::PaxosLogEntry const> get_entry(index_t index) const;

  // Sets the index of the log with the provided entry. This method requires there
  // to be no existing entry for the provided index. Otherwise, an exception is thrown.
  void set_entry(index_t index, proto::paxos::PaxosLogEntry const entry);

  // Gets the lowest available index.
  index_t next_available_index() const;

  void debug_print() const;

  std::unordered_map<index_t, proto::paxos::PaxosLogEntry const> get_log() const;

 private:
  // The Paxos Log of this node, mapping entry index to entry value.This is calculated
  // from the PaxosLearnedState by inserting a key-value pair if it has been learned.
  // We keep this as a map because entries don't have to be learned contiguously per
  // node (although recall that the Global Paxos Log cannot/won't have holes).
  std::unordered_map<index_t, proto::paxos::PaxosLogEntry const> _log;

  // Next available indices (in increasing order) in the Paxos Log. This vector contains
  // all missing indices from the _log, plus on index after the greatest populted index (or 0
  // if non are populated). We use this vector to select a paxos instance when this node want's
  // to propose a new log entry.
  std::vector<index_t> _available_indices;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSLOG_H
