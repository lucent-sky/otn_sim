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

size_t tributary_slots(OduLevel level) {
    switch (level) {
        case OduLevel::ODU1: return 1;
        case OduLevel::ODU2: return 4;
        case OduLevel::ODU3: return 16;
        case OduLevel::ODU4: return 80;
        default: return 0;
    }
}

} // namespace otn
