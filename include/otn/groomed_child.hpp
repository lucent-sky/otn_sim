#pragma once

#include <cstddef>

namespace otn {

class Odu;

struct GroomedChild {
    const Odu* child;
    std::size_t slot_width;
    std::size_t slot_offset;

    GroomedChild(const Odu* c, std::size_t width, std::size_t offset);
    GroomedChild(const Odu* c, std::size_t offset);
};

} // namespace otn
