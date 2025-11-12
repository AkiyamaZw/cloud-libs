#pragma once
#include <cassert>
#include <cstdint>


namespace cloud
{
struct Resource
{
    const char* name;
    uint64_t ref{0};
    void add_ref(){ref++;};
    void del_ref(){assert(ref != 0);ref--;};
};

}
