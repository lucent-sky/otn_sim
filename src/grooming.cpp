#include "otn/grooming.hpp"
#include <stdexcept>

namespace otn {

std::vector<GroomedChild> plan_grooming(
    OduLevel parent_level,
    const std::vector<Odu>& children
) {
    if (children.empty()) {
        throw std::runtime_error("No children to groom");
    }

    const size_t parent_slots = tributary_slots(parent_level);

    std::vector<bool> slot_map(parent_slots, false);
    std::vector<GroomedChild> result;

    // Enforce uniform child level
    OduLevel expected = children.front().level();
    for (const auto& c : children) {
        if (c.level() != expected) {
            throw std::runtime_error("Mixed ODU levels not allowed in grooming");
        }
    }

    for (const auto& child : children) {
        const size_t slots_needed = child.slots();
        bool placed = false;

        for (size_t start = 0; start + slots_needed <= parent_slots; ++start) {
            bool fits = true;

            for (size_t i = 0; i < slots_needed; ++i) {
                if (slot_map[start + i]) {
                    fits = false;
                    break;
                }
            }

            if (fits) {
                for (size_t i = 0; i < slots_needed; ++i) {
                    slot_map[start + i] = true;
                }

                result.push_back(GroomedChild{child, start});
                placed = true;
                break;
            }
        }

        if (!placed) {
            throw std::runtime_error("Unable to groom: insufficient slots");
        }
    }

    return result;
}

} // namespace otn
