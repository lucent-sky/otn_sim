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


} // namespace otn
