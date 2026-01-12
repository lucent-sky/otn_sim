#pragma once

#include "otn_types.hpp"
#include <cstddef>
#include <vector>

namespace otn {

class Odu {
public:
    // Leaf ODU (originating from client payload / OPU)
    explicit Odu(OduLevel level, size_t payload_bytes);

    // Aggregated ODU (from mux)
    Odu(OduLevel level, std::vector<Odu> children);

    OduLevel level() const;
    size_t payload_size() const;
    size_t max_capacity() const;
    bool is_aggregated() const;

private:
    OduLevel level_;
    size_t payload_bytes_;
    std::vector<Odu> children_;
};

} // namespace otn
