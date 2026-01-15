#pragma once

#include <cstdint>
#include <string>

namespace otn {

enum class OduLevel : uint8_t {
    ODU1 = 1,
    ODU2 = 2,
    ODU3 = 3,
    ODU4 = 4
};

size_t nominal_capacity(OduLevel level);

enum class MuxStatus {
    SUCCESS,
    INVALID_HIERARCHY,
    INSUFFICIENT_CAPACITY
};

struct MuxResult {
    MuxStatus status;
    std::string message;

    static MuxResult success() {
        return {MuxStatus::SUCCESS, "OK"};
    }

    static MuxResult invalid_hierarchy(const std::string& msg) {
        return {MuxStatus::INVALID_HIERARCHY, msg};
    }

    static MuxResult insufficient_capacity(const std::string& msg) {
        return {MuxStatus::INSUFFICIENT_CAPACITY, msg};
    }
};

} // namespace otn
