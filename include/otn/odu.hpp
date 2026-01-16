#pragma once

#include "otn_types.hpp"
#include <cstddef>
#include <vector>
#include "opu.hpp"

namespace otn {

class Odu {
public:
    // Leaf ODU (originating from client payload / OPU)
    explicit Odu(OduLevel level, size_t payload_bytes);

    // Leaf ODU constructed directly from an OPU
    explicit Odu(OduLevel level, const Opu& opu);

    // Aggregated ODU (from mux)
    Odu(OduLevel level, const std::vector<Odu>& children);

    OduLevel level() const;
    size_t payload_size() const;
    size_t slots() const;
    bool is_aggregated() const;

private:
    OduLevel level_;
    size_t payload_bytes_;
    size_t slot_count_;
    std::vector<Odu> children_;
};

MuxResult mux(
    OduLevel parent_level,
    const std::vector<Odu>& children,
    Odu& out_parent
);

} // namespace otn
