#include "otn/grooming_planner.hpp"
#include <stdexcept>
#include <algorithm>

namespace otn {

std::vector<GroomedChild>
plan_grooming(
    OduLevel parent_level,
    const std::vector<Odu>& children
) {
    if (children.empty()) {
        return {};
    }

    // Validate all children have same level
    OduLevel expected = children.front().level();
    for (const auto& child : children) {
        if (child.level() != expected) {
            throw std::runtime_error(
                "All children must have the same ODU level"
            );
        }
    }

    // Parent must be exactly one level higher
    uint8_t child_lvl  = static_cast<uint8_t>(expected);
    uint8_t parent_lvl = static_cast<uint8_t>(parent_level);
    if (parent_lvl != child_lvl + 1) {
        throw std::runtime_error(
            "Parent ODU level must be adjacent to children"
        );
    }

    const size_t parent_slots = tributary_slots(parent_level);

    std::vector<GroomedChild> result;
    result.reserve(children.size());

    size_t cursor = 0;

    // Simple deterministic left-packing
    for (const auto& child : children) {
        size_t child_slots = child.slots();

        if (cursor + child_slots > parent_slots) {
            throw std::runtime_error(
                "Insufficient tributary slots for grooming"
            );
        }

        result.push_back(GroomedChild{
            .child = &child,
            .slots = child_slots,
            .slot_offset = cursor
        });

        cursor += child_slots;
    }

    return result;
}

std::vector<GroomedChild>
repack_grooming(
    OduLevel parent_level,
    const std::vector<GroomedChild>& current
) {
    if (current.empty()) {
        return {};
    }

    const size_t parent_slots = tributary_slots(parent_level);

    // Validate hierarchy consistency
    OduLevel expected = current.front().child->level();
    for (const auto& g : current) {
        if (g.child->level() != expected) {
            throw std::runtime_error(
                "Mixed child ODU levels in repack"
            );
        }
    }

    uint8_t child_lvl  = static_cast<uint8_t>(expected);
    uint8_t parent_lvl = static_cast<uint8_t>(parent_level);
    if (parent_lvl != child_lvl + 1) {
        throw std::runtime_error(
            "Invalid parent/child ODU hierarchy for repack"
        );
    }

    std::vector<GroomedChild> result;
    result.reserve(current.size());

    size_t cursor = 0;

    // Stable left-compaction
    for (const auto& g : current) {
        size_t child_slots = g.slots;

        if (cursor + child_slots > parent_slots) {
            throw std::runtime_error(
                "Repacked grooming exceeds parent capacity"
            );
        }

        result.push_back(GroomedChild{
            .child = g.child,
            .slots = child_slots,
            .slot_offset = cursor
        });

        cursor += child_slots;
    }

    return result;
}


} // namespace otn
