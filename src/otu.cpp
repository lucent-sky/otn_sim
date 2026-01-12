#include "otn/otu.hpp"

namespace otn {

Otu::Otu(const Odu& odu, bool fec_enabled)
    : odu_(odu), fec_enabled_(fec_enabled)
{}

bool Otu::fec_enabled() const {
    return fec_enabled_;
}

uint8_t Otu::odu_level() const {
    return odu_.level();
}

size_t Otu::payload_size() const {
    return odu_.payload_size();
}

} // namespace otn
