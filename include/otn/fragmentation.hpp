#pragma once

#include "groomed_child.hpp"
#include "odu.hpp"

#include <vector>
#include <cstddef>

namespace otn {

struct FragmentationMetrics {
    std::size_t gap_count;
    std::size_t total_gap_slots;
    std::size_t max_gap;
    std::size_t span_slots;
    double utilization;
};

struct FragmentationCostWeights {
    double utilization_weight = 1.0;
    double gap_count_weight   = 0.3;
    double gap_slots_weight   = 0.5;
    double max_gap_weight     = 0.2;
};

FragmentationMetrics analyze_fragmentation(const std::vector<GroomedChild>& grooming);

double fragmentation_cost(
    const FragmentationMetrics& metrics,
    const FragmentationCostWeights& weights = {}
);

std::vector<GroomedChild> repack_grooming(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
);

std::vector<GroomedChild> repack_grooming_deterministic(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
);

} // namespace otn
