#pragma once

#include <cstddef>
#include <vector>
#include "otn/groomed_child.hpp"
#include "otn/otn_types.hpp"

namespace otn {

struct FragmentationMetrics {
    size_t gap_count;
    size_t total_gap_slots;
    size_t max_gap;
    size_t span_slots;
    double utilization;
};

FragmentationMetrics analyze_fragmentation(
    const std::vector<GroomedChild>& grooming
);

std::vector<GroomedChild> repack_grooming(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
);

std::vector<GroomedChild> repack_grooming_size_aware(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
);

} // namespace otn
