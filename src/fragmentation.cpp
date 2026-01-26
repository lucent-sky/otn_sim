#include "otn/fragmentation.hpp"
#include "otn/odu.hpp"

#include <algorithm>
#include <stdexcept>

namespace otn {

FragmentationMetrics analyze_fragmentation(
    const std::vector<GroomedChild>& grooming
) {
    if (grooming.empty()) {
        return {0, 0, 0, 0, 0.0};
    }

    // Work on a sorted copy
    std::vector<GroomedChild> sorted = grooming;
    std::sort(sorted.begin(), sorted.end(),
        [](const GroomedChild& a, const GroomedChild& b) {
            return a.slot_offset < b.slot_offset;
        }
    );

    size_t gap_count = 0;
    size_t total_gap_slots = 0;
    size_t max_gap = 0;

    size_t first_slot = sorted.front().slot_offset;
    size_t last_slot =
        sorted.front().slot_offset + sorted.front().slot_width - 1;

    for (size_t i = 1; i < sorted.size(); ++i) {
        size_t prev_end =
            sorted[i - 1].slot_offset +
            sorted[i - 1].slot_width - 1;

        if (sorted[i].slot_offset > prev_end + 1) {
            size_t gap = sorted[i].slot_offset - (prev_end + 1);
            ++gap_count;
            total_gap_slots += gap;
            max_gap = std::max(max_gap, gap);
        }

        last_slot = std::max(
            last_slot,
            sorted[i].slot_offset + sorted[i].slot_width - 1
        );
    }

    size_t span_slots = last_slot - first_slot + 1;

    const size_t occupied_slots =
        span_slots - total_gap_slots;

    double utilization =
        span_slots == 0
            ? 0.0
            : static_cast<double>(occupied_slots) /
              static_cast<double>(span_slots);

    return {
        gap_count,
        total_gap_slots,
        max_gap,
        span_slots,
        utilization
    };
}


std::vector<GroomedChild> repack_grooming_size_aware(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
) {
    if (grooming.empty()) return {};

    std::vector<GroomedChild> sorted = grooming;

    // size-aware:
    // prefer larger slot_width
    // if equal, prefer larger payload size
    std::stable_sort(sorted.begin(), sorted.end(),
        [](const GroomedChild& a, const GroomedChild& b) {
            if (a.slot_width != b.slot_width)
                return a.slot_width > b.slot_width;
            return a.child->payload_size() > b.child->payload_size();
        }
    );

    size_t max_slots = tributary_slots(parent_level);
    std::vector<bool> slot_map(max_slots, false);
    std::vector<GroomedChild> repacked;

    for (const auto& g : sorted) {
        bool placed = false;
        for (size_t start = 0; start + g.slot_width <= max_slots; ++start) {
            bool free = true;
            for (size_t i = 0; i < g.slot_width; ++i) {
                if (slot_map[start + i]) {
                    free = false;
                    break;
                }
            }
            if (free) {
                for (size_t i = 0; i < g.slot_width; ++i)
                    slot_map[start + i] = true;

                repacked.emplace_back(g.child, g.slot_width, start);
                placed = true;
                break;
            }
        }

        if (!placed) {
            throw std::runtime_error("Cannot repack: not enough contiguous slots");
        }
    }

    return repacked;
}

std::vector<GroomedChild> repack_grooming(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
) {
    return repack_grooming_size_aware(parent_level, grooming);
}

std::vector<GroomedChild> repack_grooming_deterministic(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
) {
    if (grooming.empty()) return {};

    size_t max_slots = tributary_slots(parent_level);

    // Step 1: Sort children descending by slot_width, preserve original order on ties
    std::vector<GroomedChild> sorted = grooming;
    std::stable_sort(sorted.begin(), sorted.end(),
        [](const GroomedChild& a, const GroomedChild& b) {
            return a.slot_width > b.slot_width;
        }
    );

    // Step 2: Greedy placement: place each child in first available contiguous slot
    std::vector<bool> slot_map(max_slots, false); // marks used slots
    std::vector<GroomedChild> repacked;

    for (const auto& g : sorted) {
        // Find first contiguous space of size g.slot_width
        size_t start = 0;
        bool placed = false;
        while (start + g.slot_width <= max_slots) {
            bool free = true;
            for (size_t i = 0; i < g.slot_width; ++i) {
                if (slot_map[start + i]) { free = false; break; }
            }
            if (free) {
                // Place child here
                for (size_t i = 0; i < g.slot_width; ++i) slot_map[start + i] = true;
                repacked.push_back({g.child, g.slot_width, start});
                placed = true;
                break;
            }
            ++start;
        }

        if (!placed) {
            throw std::runtime_error("Cannot repack: not enough contiguous slots");
        }
    }

    return repacked;
}

double fragmentation_cost(
    const FragmentationMetrics& m,
    const FragmentationCostWeights& w
) {
    if (m.span_slots == 0) return 0.0;

    const double utilization_penalty = 1.0 - m.utilization;
    const double gap_slots_ratio =
        static_cast<double>(m.total_gap_slots) / m.span_slots;
    const double max_gap_ratio =
        static_cast<double>(m.max_gap) / m.span_slots;

    return
        w.utilization_weight * utilization_penalty +
        w.gap_count_weight   * static_cast<double>(m.gap_count) +
        w.gap_slots_weight   * gap_slots_ratio +
        w.max_gap_weight     * max_gap_ratio;
}

} // namespace otn
