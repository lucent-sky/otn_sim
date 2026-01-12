#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>

namespace otn {

class Payload {
public:
    explicit Payload(size_t size);

    size_t size() const;

private:
    std::vector<uint8_t> data_;
};

} // namespace otn
