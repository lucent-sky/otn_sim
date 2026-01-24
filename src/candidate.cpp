#include "otn/grooming_planner.hpp"
#include "otn/candidate.hpp"
#include "otn/fragmentation.hpp"

#include <stdexcept>

namespace otn {

std::vector<GroomedChild>
admit_candidates(
    OduLevel parent_level,
    std::vector<GroomedChild> current,
    const std::vector<Candidate>& candidates
) {
    for (const auto& cand : candidates) {
        if (!cand.child) {
            throw std::runtime_error("Null candidate child");
        }

        // Find feasible offsets given current grooming
        const auto offsets =
            feasible_offsets(parent_level, current, *cand.child);

        if (offsets.empty()) {
            // Minimal policy: reject silently
            // Later: return AdmissionResult, log, or throw
            continue;
        }

        // Greedy: choose leftmost offset
        const std::size_t chosen = offsets.front();

        current.emplace_back(cand.child, chosen);
    }

    return current;
}

} // namespace otn
