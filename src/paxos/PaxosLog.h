#ifndef UNI_PAXOS_PAXOSLOG_H
#define UNI_PAXOS_PAXOSLOG_H

#include <functional>
#include <unordered_map>

#include <boost/optional.hpp>

#include <common/common.h>
#include <paxos/PaxosTypes.h>
#include <proto/paxos.pb.h>

namespace uni {
namespace paxos {

// The log of values that an instance of the MultiPaxos algorithms tries to add
class PaxosLog {
 public:
  PaxosLog();

  // Builds a complete PaxosLog, given a log.
  PaxosLog(std::unordered_map<index_t, proto::paxos::PaxosLogEntry> log);

  // Returns the entry at the index if it exists, otherwise returns an empty optional
  boost::optional<proto::paxos::PaxosLogEntry const> get_entry(index_t index) const;

  // Sets the index of the log with the provided entry. If an entry already exists,
  // for the index, then the method just returns.
  void set_entry(index_t index, proto::paxos::PaxosLogEntry const entry);

  // Add callback to the list of callbacks to invoke when an entry is added.
  void add_callback(proto::paxos::PaxosLogEntry::EntryContentCase content_case, std::function<void(index_t, proto::paxos::PaxosLogEntry)> callback);

  // Gets the lowest available index.
  index_t next_available_index() const;

  std::vector<index_t> get_available_indices() const;

  std::unordered_map<index_t, proto::paxos::PaxosLogEntry const> const& get_log()  const;

  std::string debug_string() const;

 private:
  // The Paxos Log of this node, mapping entry index to entry value.This is calculated
  // from the PaxosLearnedState by inserting a key-value pair if it has been learned.
  // We keep this as a map because entries don't have to be learned contiguously per
  // node (although recall that the Global Paxos Log cannot/won't have holes).
  std::unordered_map<index_t, proto::paxos::PaxosLogEntry const>  _log;

  // Next available indices (in increasing order) in the Paxos Log. This vector contains
  // all missing indices from the _log, plus on index after the greatest populted index (or 0
  // if non are populated). We use this vector to select a paxos instance when this node want's
  // to propose a new log entry.
  std::vector<index_t> _available_indices;

  // Callbacks to invoke everytime a new entry is entered into the PaxosLog.
  // Note that we don't invoke these for entries that aren't yet connected to
  // all contiguously received entries.
  // 
  // We index each list of callbacks by type of the PaxosLogEntry, since almost all
  // callbacks usually handle only type (and even if a callback handles multiple
  // types, they can be split up for each type)
  std::unordered_map<
    proto::paxos::PaxosLogEntry::EntryContentCase,
    std::vector<std::function<void(index_t, proto::paxos::PaxosLogEntry)>>
  > _callbacks;
};

} // namespace paxos
} // namespace uni


#endif // UNI_PAXOS_PAXOSLOG_H
