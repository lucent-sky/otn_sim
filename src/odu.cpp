#include "otn/odu.hpp"
#include <stdexcept>

namespace otn {

namespace {

// placeholder values for now
size_t capacity_for_level(OduLevel level) {
    switch (level) {
        case OduLevel::ODU1: return 2500;
        case OduLevel::ODU2: return 10000;
        case OduLevel::ODU4: return 100000;
        default: return 0;
    }
}

} // anonymous namespace

Odu::Odu(OduLevel level, size_t payload_bytes)
    : level_(level),
      payload_bytes_(payload_bytes)
{
    if (payload_bytes_ > nominal_capacity(level_)) {
        throw std::runtime_error("ODU payload exceeds nominal capacity");
    }
}

Odu::Odu(OduLevel level, std::vector<Odu> children)
    : level_(level),
      children_(std::move(children))
{
    size_t sum = 0;
    for (const auto& c : children_) {
        sum += c.payload_size();
    }

    if (sum > nominal_capacity(level_)) {
        throw std::runtime_error("Aggregated ODU exceeds nominal capacity");
    }

    payload_bytes_ = sum;
}

Odu::Odu(OduLevel level, const Opu& opu)
    : level_(level),
      payload_bytes_(opu.payload_size())
{}

OduLevel Odu::level() const {
    return level_;
}

size_t Odu::payload_size() const {
    return payload_bytes_;
}

size_t Odu::max_capacity() const {
    return capacity_for_level(level_);
}

bool Odu::is_aggregated() const {
    return !children_.empty();
}

MuxResult mux(
    OduLevel parent_level,
    const std::vector<Odu>& children,
    Odu& out_parent
) {
    if (children.empty()) {
        return MuxResult::invalid_hierarchy("Can't mux without children!");
    }

    size_t total_payload = 0;

    for (const auto& child : children) {
        if (static_cast<uint8_t>(child.level()) >=
            static_cast<uint8_t>(parent_level)) {
            return MuxResult::invalid_hierarchy(
                "Parent level is higher than child - can't mux"
            );
        }
        total_payload += child.payload_size();
    }

    size_t capacity = nominal_capacity(parent_level);
    if (total_payload > capacity) {
        return MuxResult::insufficient_capacity(
            "Aggregated payload is above nominal capacity"
        );
    }

    out_parent = Odu(parent_level, children);
    return MuxResult::success();
}

} // namespace otn
