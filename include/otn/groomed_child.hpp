#pragma once

#include <cstddef>

namespace otn {

class Odu;

/**
 * Represents an explicitly groomed child ODU placed
 * at a specific tributary slot offset.
 */
struct GroomedChild {
    const Odu* child;
    std::size_t slot_width;
    std::size_t slot_offset;

    // Constructor: slot_width passed explicitly
    GroomedChild(const Odu* c, std::size_t width, std::size_t offset)
        : child(c), slot_width(width), slot_offset(offset) {}
};

} // namespace otn
