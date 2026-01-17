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
};

} // namespace otn
