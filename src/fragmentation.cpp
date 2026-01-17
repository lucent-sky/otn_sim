#include "otn/fragmentation.hpp"
#include "otn/odu.hpp"

#include <algorithm>
#include <stdexcept>

namespace otn {

FragmentationMetrics analyze_fragmentation(const std::vector<GroomedChild>& grooming) {
    if (grooming.empty()) {
        return {0, 0, 0, 0, 0.0};  
    }

    size_t gap_count = 0;
    size_t total_gap_slots = 0;
    size_t max_gap = 0;
    size_t first_slot = grooming.front().slot_offset;
    size_t last_slot = grooming.front().slot_offset + grooming.front().child->slots() - 1;

    // Compute span and gaps
    for (size_t i = 1; i < grooming.size(); ++i) {
        size_t prev_end = grooming[i-1].slot_offset + grooming[i-1].child->slots() - 1;
        size_t gap = 0;

        if (grooming[i].slot_offset > prev_end + 1) {
            gap = grooming[i].slot_offset - (prev_end + 1);
            ++gap_count;
            total_gap_slots += gap;
            if (gap > max_gap) max_gap = gap;
        }

        last_slot = std::max(last_slot, grooming[i].slot_offset + grooming[i].child->slots() - 1);
    }

    size_t span_slots = last_slot - first_slot + 1;
    double utilization = span_slots == 0 ? 0.0 : double(span_slots - total_gap_slots) / double(span_slots);

    return {gap_count, total_gap_slots, max_gap, span_slots, utilization};
}

std::vector<GroomedChild> repack_grooming(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
) {
    return repack_grooming_size_aware(parent_level, grooming);
}

std::vector<GroomedChild> repack_grooming_size_aware(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
) {
    if (grooming.empty()) {
        return {};
    }

    std::vector<GroomedChild> sorted = grooming;

    // Sort by descending slot width (size-aware)
    std::sort(sorted.begin(), sorted.end(),
        [](const GroomedChild& a, const GroomedChild& b) {
            return a.slot_width > b.slot_width;
        }
    );

    std::vector<GroomedChild> repacked;
    std::size_t current_slot = 0;
    std::size_t max_slots = tributary_slots(parent_level);

    for (const auto& g : sorted) {
        if (current_slot + g.slot_width > max_slots) {
            throw std::runtime_error("Repacking exceeds parent capacity");
        }

        repacked.push_back({
            g.child,
            g.slot_width,
            current_slot
        });

        current_slot += g.slot_width;
    }

    return repacked;
}

} // namespace otn
