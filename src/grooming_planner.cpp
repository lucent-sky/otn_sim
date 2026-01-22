#include "otn/grooming_planner.hpp"
#include "otn/odu.hpp"
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

        result.emplace_back(&child, cursor);

        cursor += child_slots;
    }

    return result;
}

/*
 * NOTE: reimplemented in fragmentation.cpp
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
        size_t child_slots = g.slot_width;

        if (cursor + child_slots > parent_slots) {
            throw std::runtime_error(
                "Repacked grooming exceeds parent capacity"
            );
        }

        result.emplace_back(g.child, cursor);

        cursor += child_slots;
    }

    return result;
}

std::vector<GroomedChild>
repack_grooming_size_aware(
    OduLevel parent_level,
    const std::vector<GroomedChild>& current
) {
    if (current.empty()) {
        return {};
    }

    const size_t parent_slots = tributary_slots(parent_level);

    // Copy and sort by descending size
    std::vector<GroomedChild> sorted = current;
    std::stable_sort(
        sorted.begin(),
        sorted.end(),
        [](const auto& a, const auto& b) {
            return a.slot_width > b.slot_width;
        }
    );

    std::vector<GroomedChild> result;
    result.reserve(sorted.size());

    size_t cursor = 0;

    for (const auto& g : sorted) {
        if (cursor + g.slot_width > parent_slots) {
            throw std::runtime_error(
                "Size-aware repack exceeds parent capacity"
            );
        }

        result.emplace_back(g.child, cursor);

        cursor += g.slot_width;
    }

    return result;
}
    */

std::vector<bool> occupied_slots(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming
) {
    const std::size_t max_slots = tributary_slots(parent_level);
    std::vector<bool> slots(max_slots, false);

    for (const auto& g : grooming) {
        const std::size_t start = g.slot_offset;
        const std::size_t width = g.slot_width;

        if (start + width > max_slots) {
            throw std::runtime_error("GroomedChild exceeds parent slot capacity");
        }

        for (std::size_t i = 0; i < width; ++i) {
            if (slots[start + i]) {
                throw std::runtime_error("Overlapping GroomedChild slots detected");
            }
            slots[start + i] = true;
        }
    }

    return slots;
}

std::vector<std::size_t> feasible_offsets(
    OduLevel parent_level,
    const std::vector<GroomedChild>& grooming,
    const Odu& candidate
) {
    const std::size_t max_slots = tributary_slots(parent_level);
    const std::size_t width = tributary_slots(candidate.level());

    if (width > max_slots) {
        return {}; // candidate can never fit
    }

    const std::vector<bool> occupied =
        occupied_slots(parent_level, grooming);

    std::vector<std::size_t> offsets;

    // Left-to-right scan
    for (std::size_t start = 0; start + width <= max_slots; ++start) {
        bool fits = true;

        for (std::size_t i = 0; i < width; ++i) {
            if (occupied[start + i]) {
                fits = false;
                break;
            }
        }

        if (fits) {
            offsets.push_back(start);
        }
    }

    return offsets;
}

} // namespace otn
