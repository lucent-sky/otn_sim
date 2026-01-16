#pragma once

#include "otn_types.hpp"
#include <cstddef>
#include <vector>
#include "opu.hpp"

namespace otn {

class Odu;  

struct GroomedChild {
    const Odu* child;
    size_t slots;
    size_t slot_offset;
};

class Odu {
public:
    // Leaf ODU (originating from client payload / OPU)
    explicit Odu(OduLevel level, size_t payload_bytes);

    // Leaf ODU constructed directly from an OPU
    explicit Odu(OduLevel level, const Opu& opu);

    /*
    DEPRECATED!! Implicit aggregation has been phased out in favor of explicit grooming
    Odu(OduLevel level, const std::vector<Odu>& children);
    */

    OduLevel level() const;
    size_t payload_size() const;
    size_t slots() const;
    bool is_aggregated() const;
    Odu(OduLevel level, std::vector<GroomedChild> groomed_children); //grooming constructor (mandatory)
    const std::vector<GroomedChild>& groomed_children() const; //grooming introspection

private:
    OduLevel level_;
    size_t payload_bytes_;
    size_t slot_count_;
    std::vector<GroomedChild> groomed_children_;
};

MuxResult mux(
    OduLevel parent_level,
    //const std::vector<Odu>& children, DEPRECATED for grooming
    const std::vector<GroomedChild>& groomed_children,
    Odu& out_parent
);

} // namespace otn
