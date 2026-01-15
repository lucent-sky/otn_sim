#include "otn/otn_types.hpp"

namespace otn {

size_t nominal_capacity(OduLevel level) {
    switch (level) {
        case OduLevel::ODU1:
            return 2500;
        case OduLevel::ODU2:
            return 10000;
        case OduLevel::ODU3:
            return 40000;
        case OduLevel::ODU4:
            return 100000;
        default:
            return 0; // throws 0 if undefined OTN level
    }
}

} // namespace otn
