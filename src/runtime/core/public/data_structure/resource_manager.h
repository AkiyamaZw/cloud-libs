#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>

namespace cloud
{
struct Resource
{
    std::string_view name;
    uint64_t ref{0};
    void add_ref(){ref++;};
    void del_ref(){assert(ref != 0);ref--;};
};

}
