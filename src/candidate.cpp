#include "otn/grooming_planner.hpp"
#include "otn/candidate.hpp"
#include "otn/fragmentation.hpp"

#include <stdexcept>
#include <limits>

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

        const auto offsets =
            feasible_offsets(parent_level, current, *cand.child);

        if (offsets.empty()) {
            continue; // cannot admit this candidate
        }

        double best_cost = std::numeric_limits<double>::infinity();
        std::size_t best_offset = 0;
        bool found = false;

        for (std::size_t offset : offsets) {
            // Simulate placement
            std::vector<GroomedChild> trial = current;
            trial.emplace_back(cand.child, offset);

            auto metrics = analyze_fragmentation(trial);
            const double cost  = fragmentation_cost(metrics);

            if (cost < best_cost) {
                best_cost = cost;
                best_offset = offset;
                found = true;
            }
        }

        if (found) {
            current.emplace_back(cand.child, best_offset);
        }
    }

    return current;
}

} // namespace otn
