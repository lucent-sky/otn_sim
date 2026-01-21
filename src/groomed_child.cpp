#include "otn/groomed_child.hpp"
#include "otn/odu.hpp"

namespace otn {

GroomedChild::GroomedChild(const Odu* c,
                           std::size_t width,
                           std::size_t offset)
    : child(c), slot_width(width), slot_offset(offset) {}

GroomedChild::GroomedChild(const Odu* c,
                           std::size_t offset)
    : GroomedChild(c, c->slots(), offset) {}

} // namespace otn
