#include "otn/odu.hpp"

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
{}

Odu::Odu(OduLevel level, std::vector<Odu> children)
    : level_(level),
      children_(std::move(children))
{
    payload_bytes_ = 0;
    for (const auto& child : children_) {
        payload_bytes_ += child.payload_size();
    }
}

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

} // namespace otn
