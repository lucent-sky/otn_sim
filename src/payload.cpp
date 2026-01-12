#include "otn/payload.hpp"
#include <cstddef>

namespace otn {

Payload::Payload(size_t size)
    : data_(size, 0)
{}

size_t Payload::size() const {
    return data_.size();
}

}
