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

// ---------------- AGGREGATED ODU ----------------

Odu::Odu(OduLevel level, const std::vector<Odu>& children)
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
    return !children_.empty();
}

// ---------------- MUX ----------------

MuxResult mux(
    OduLevel parent_level,
    const std::vector<Odu>& children,
    Odu& out_parent
) {
    if (children.empty()) {
        return MuxResult::invalid_hierarchy("Can't mux without children");
    }

    OduLevel expected = children.front().level();
    for (const auto& child : children) {
        if (child.level() != expected) {
            return MuxResult::invalid_hierarchy(
                "All children must have same ODU level"
            );
        }
    }

    size_t total_slots = 0;

    for (const auto& child : children) {
        uint8_t child_lvl  = static_cast<uint8_t>(child.level());
        uint8_t parent_lvl = static_cast<uint8_t>(parent_level);

        // Parent must be exactly one level higher
        if (parent_lvl != child_lvl + 1) {
            return MuxResult::invalid_hierarchy(
                "ODU levels must be adjacent"
            );
        }

        total_slots += child.slots();
    }

    if (total_slots > tributary_slots(parent_level)) {
        return MuxResult::insufficient_capacity(
            "Aggregated tributary slots exceed parent capacity"
        );
    }

    out_parent = Odu(parent_level, children);
    return MuxResult::success();
}

} // namespace otn
