#include "otn/opu.hpp"

namespace otn {

Opu::Opu(const Payload& payload)
    : payload_(payload)
{}

size_t Opu::payload_size() const {
    return payload_.size();
}

}
