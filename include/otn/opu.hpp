#pragma once
#include "payload.hpp"

namespace otn {

class Opu {
public:
    explicit Opu(const Payload& payload);

    size_t payload_size() const;

private:
    Payload payload_;
};

}
