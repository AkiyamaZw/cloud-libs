#pragma once
#include "define.h"
#include <cstdint>
#include <typeinfo>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace cloud::world::ecs
{
// single component meta records
struct MetaType
{
    using ConstructFunc = void(void *);
    using DestructFunc = void(void *);
    const char *name{"null"};
    ConstructFunc *constructor{nullptr};
    DestructFunc *destructor{nullptr};
    size_t hash;
    uint16_t size{0};
    uint16_t align{0};

    bool is_empty() const { return align == 0; }

    template <typename T>
    static const std::type_info &build_info()
    {
        return typeid(T);
    }

    template <typename T>
    static constexpr MetaType build()
    {
        MetaType meta{};
        const std::type_info &type = build_info<T>();
        meta.hash = type.hash_code();
        meta.name = type.name();

        if constexpr (std::is_empty_v<T>)
        {
            meta.align = 0;
            meta.size = 0;
        }
        else
        {
            meta.align = alignof(T);
            meta.size = sizeof(T);
        }
        meta.constructor = [](void *p) { new (p) T{}; };
        meta.destructor = [](void *p) { ((T *)p)->~T(); };
        return meta;
    }
};

} // namespace cloud::world::ecs