#include "otn/grooming_planner.hpp"
#include "otn/candidate.hpp"
#include "otn/fragmentation.hpp"

#include <stdexcept>
#include <limits>
#include <unordered_map>
#include <optional>

namespace otn {

std::vector<GroomedChild>
admit_candidates(
    OduLevel parent_level,
    std::vector<GroomedChild> current,
    const std::vector<Candidate>& candidates
) {
    std::unordered_map<const Odu*, std::vector<const Candidate*>> by_child;
    std::vector<const Odu*> child_order;

    // Accumulate children to add after processing all candidates
    std::vector<GroomedChild> to_add;

    // Group candidates by child, preserving first-seen order
    for (const auto& c : candidates) {
        if (!c.child) {
            throw std::runtime_error("Null candidate child");
        }

        auto& group = by_child[c.child];
        if (group.empty()) {
            child_order.push_back(c.child);
        }
        group.push_back(&c);
    }

    // Process children in stable order
    for (const Odu* child : child_order) {
        const auto& group = by_child[child];

        double best_cost = std::numeric_limits<double>::infinity();
        std::optional<std::size_t> best_offset;

        // Evaluate all candidate placements for this child
        for (const Candidate* cand : group) {
            const auto offsets = feasible_offsets(parent_level, current, *child);

            for (std::size_t offset : offsets) {
                if (offset != cand->offset) continue;

                std::vector<GroomedChild> trial = current;
                // Include previously selected children in to_add to evaluate fragmentation correctly
                trial.insert(trial.end(), to_add.begin(), to_add.end());
                trial.emplace_back(child, offset);

                double cost = fragmentation_cost(analyze_fragmentation(trial));

                // preserves greedy + stable tie-breaking
                if (
                    cost < best_cost ||
                    (cost == best_cost && (!best_offset.has_value() || offset < *best_offset))
                ) {
                    best_cost = cost;
                    best_offset = offset;
                }
            }
        }

        // Admit once per child into to_add (do not modify current yet)
        if (best_offset.has_value()) {
            to_add.emplace_back(child, *best_offset);
        }
    }

    // Append all admitted children at the end to preserve input order
    current.insert(current.end(), to_add.begin(), to_add.end());
    return current;
}

} // namespace otn
