#include "otn/odu.hpp"
#include <stdexcept>

namespace otn {

namespace {

/*
DEPRECATED FUNCTION: used for payload-based capacity model

size_t capacity_for_level(OduLevel level) {
    switch (level) {
        case OduLevel::ODU1: return 2500;
        case OduLevel::ODU2: return 10000;
        case OduLevel::ODU4: return 100000;
        default: return 0;
    }
}
*/

} // anonymous namespace

// ---------------- LEAF ODU ----------------

Odu::Odu(OduLevel level, size_t payload)
    : level_(level),
      payload_bytes_(payload),
      slot_count_(tributary_slots(level))
{
    if (payload > nominal_capacity(level)) {
        throw std::runtime_error("ODU payload exceeds nominal capacity");
    }
}

// ---------------- OPU → ODU ----------------

Odu::Odu(OduLevel level, const Opu& opu)
    : level_(level),
      payload_bytes_(opu.payload_size()),
      slot_count_(tributary_slots(level))
{}

// DEPRECATED FOR GROOMING MODEL ---------------- AGGREGATED ODU ----------------

/* Odu::Odu(OduLevel level, const std::vector<Odu>& children)
    : level_(level),
      payload_bytes_(0),
      slot_count_(0),
      children_(children)
{
    for (const auto& child : children_) {
        payload_bytes_ += child.payload_size();
        slot_count_   += child.slots();
    }

    if (slot_count_ > tributary_slots(level)) {
        throw std::runtime_error("ODU tributary slot overflow");
    }
} */

// ---------------- AGGREGATED ODU w/EXPLICIT GROOMING ----------------

Odu::Odu(OduLevel level, std::vector<GroomedChild> groomed)
    : level_(level),
      payload_bytes_(0),
      slot_count_(0),
      groomed_children_(std::move(groomed))
{
    const size_t parent_slots = tributary_slots(level_);
    std::vector<bool> slot_map(parent_slots, false);

    for (const auto& gc : groomed_children_) {
        const Odu& child = gc.child;
        const size_t offset = gc.slot_offset;
        const size_t child_slots = child.slots();

        // Child must be exactly one level lower
        uint8_t child_lvl  = static_cast<uint8_t>(child.level());
        uint8_t parent_lvl = static_cast<uint8_t>(level_);
        if (parent_lvl != child_lvl + 1) {
            throw std::runtime_error("Invalid ODU level hierarchy");
        }

        // Bounds check
        if (offset + child_slots > parent_slots) {
            throw std::runtime_error("Groomed child exceeds parent slot range");
        }

        // Overlap check
        for (size_t i = 0; i < child_slots; ++i) {
            if (slot_map[offset + i]) {
                throw std::runtime_error("Overlapping tributary slots");
            }
            slot_map[offset + i] = true;
        }

        slot_count_   += child_slots;
        payload_bytes_ += child.payload_size();
    }
}

// ---------------- ACCESSORS ----------------

OduLevel Odu::level() const {
    return level_;
}

size_t Odu::payload_size() const {
    return payload_bytes_;
}

size_t Odu::slots() const {
    return slot_count_;
}

bool Odu::is_aggregated() const {
    return !groomed_children_.empty();
}

// ---------------- MUX ----------------

MuxResult mux(
    OduLevel parent_level,
    const std::vector<GroomedChild>& groomed_children,
    Odu& out_parent
) {
    if (groomed_children.empty()) {
        return MuxResult::invalid_hierarchy("Can't mux without children");
    }

    // all children must be same level
    OduLevel expected = groomed_children.front().child.level();
    for (const auto& gc : groomed_children) {
        if (gc.child.level() != expected) {
            return MuxResult::invalid_hierarchy(
                "All children must have same ODU level"
            );
        }
    }

    // parent must be **exactly** one level higher
    uint8_t child_lvl  = static_cast<uint8_t>(expected);
    uint8_t parent_lvl = static_cast<uint8_t>(parent_level);
    if (parent_lvl != child_lvl + 1) {
        return MuxResult::invalid_hierarchy(
            "ODU levels must be adjacent"
        );
    }

    // construction enforces slot placement + overlap rules
    try {
        out_parent = Odu(parent_level, groomed_children);
    } catch (const std::exception& e) {
        return MuxResult::insufficient_capacity(e.what());
    }

    return MuxResult::success();
}

} // namespace otn
