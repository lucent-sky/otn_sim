#pragma once

#include "otn_types.hpp"
#include "odu.hpp"

#include <vector>

namespace otn {

class OduMux {
public:
    explicit OduMux(OduLevel target_level);

    MuxResult add_client(const Odu& client);
    bool can_accept(const Odu& client) const;

    bool is_full() const;
    size_t used_capacity() const;
    size_t remaining_capacity() const;

    Odu multiplex() const;
    void reset();

private:
    bool is_valid_client(const Odu& client) const;
    size_t capacity_for_level(OduLevel level) const;

private:
    OduLevel target_level_;
    size_t max_capacity_;
    size_t used_capacity_;
    std::vector<Odu> clients_;
};

} // namespace otn
