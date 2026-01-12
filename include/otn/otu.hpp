#pragma once
#include "odu.hpp"

namespace otn {

class Otu {
public:
    Otu(const Odu& odu, bool fec_enabled);

    bool fec_enabled() const;
    OduLevel odu_level() const;
    size_t payload_size() const;

private:
    Odu odu_;
    bool fec_enabled_;
};

}
