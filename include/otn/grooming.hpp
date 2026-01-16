#pragma once

#include "odu.hpp"
#include <vector>

namespace otn {

// Deterministic first-fit grooming planner
std::vector<GroomedChild> plan_grooming(
    OduLevel parent_level,
    const std::vector<Odu>& children
);

} // namespace otn
