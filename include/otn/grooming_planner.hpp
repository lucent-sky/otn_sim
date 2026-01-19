#pragma once

#include "otn/odu.hpp"
#include <vector>

namespace otn {

/*
 *  - Simple grooming planner
 *  - Performs deterministic left-packing
 *  - Throws on capacity overflow
 */

std::vector<GroomedChild>
plan_grooming(
    OduLevel parent_level,
    const std::vector<Odu>& children
);

/*
 *  - Fragmentation-aware repacking
 *  - Accepts an existing grooming layout
 *  - Reassigns slot offsets to eliminate gaps
 *  - Preserves child ordering
 */
std::vector<GroomedChild>
repack_grooming(
    OduLevel parent_level,
    const std::vector<GroomedChild>& current
);

/*
 *  - Size-aware repacking
 *  - Reorders children by descending slot size
 *  - Packs largest first to reduce fragmentation
 */
std::vector<GroomedChild>
repack_grooming_size_aware(
    OduLevel parent_level,
    const std::vector<GroomedChild>& current
);

/*
 *  - Marks slots as open or closed based on whether child occupies them
 */
std::vector<bool> occupied_slots(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
);

/*
 *  - Returns all slot offsets where candidate can be placed
 *  - No overlaps and within parent capacity
 */
std::vector<size_t> feasible_offsets(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming,
    const Odu& candidate
);

struct AdmissionResult {
    bool admitted;
    size_t chosen_offset;
    double cost;
};

} // namespace otn
