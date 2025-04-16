#pragma once
#include <unordered_map>
#include <limits>
#include "meta_type.h"
#include <utility>
#include "define.h"

namespace cloud::world::ecs
{

/**
 * @brief a component meta info, index management
 *
 */

struct ComponentManagerData
{
    static constexpr uint64_t INVALID_INDEX_ID =
        std::numeric_limits<uint64_t>::max();

    ~ComponentManagerData();

    // a component unique id generator
    uint64_t next_index_{1};

    // hash to unique id mapping
    std::unordered_map<size_t, uint64_t> type_to_index_{};

    // hash to meta type mapping
    std::unordered_map<uint64_t, MetaType> metatype_map_{};

    // container for singleton component
    std::unordered_map<size_t, void *> singleton_components_{};
};

namespace Component
{
// get a unique id for component
template <typename T>
size_t get_id(const ComponentManagerData &data)
{
    auto it = data.type_to_index_.find(typeid(T).hash_code());
    if (it != data.type_to_index_.end())
    {
        return it->second;
    }
    return ComponentManagerData::INVALID_INDEX_ID;
}

template <typename T>
size_t get_or_register(ComponentManagerData &data)
{
    auto index = get_id<T>(data);
    if (index != -1)
    {
        return index;
    }
    size_t new_index = data.next_index_++;
    data.type_to_index_[typeid(T).hash_code()] = new_index;
    return new_index;
}

template <typename T>
const MetaType *get_metatype(ComponentManagerData &data)
{
    const auto &info = MetaType::build_info<T>();
    size_t hash = info.hash_code();
    auto type = data.metatype_map_.find(hash);
    if (type == data.metatype_map_.end())
    {
        data.metatype_map_.emplace(hash, MetaType::build<T>());
    }
    return &data.metatype_map_[hash];
}

template <typename... C>
MetaTypeList get_metatypes(ComponentManagerData &data)
{
    const MetaType *types[] = {get_metatype<C>(data)...};
    size_t num = (sizeof(types) / sizeof(*types));
    return {types, types + num};
}

template <typename... C>
MetaTypeList get_info_for_archetype(ComponentManagerData &data, MaskType &mask)
{
    (..., (mask.set(get_or_register<C>(data))));

    return get_metatypes<C...>(data);
}

template <typename C>
C *get_singleton_component(ComponentManagerData &data)
{
    constexpr size_t hash = MetaType::build_info<C>().hash_code();
    auto lookup = data.singleton_components_.find(hash);

    return lookup != data.singleton_components_.end() ? (C *)(lookup->second)
                                                      : nullptr;
}

template <typename C>
C *set_singleton_component(ComponentManagerData &data)
{
    constexpr size_t hash = MetaType::build_info<C>().hash_code();
    C *old_singleton = get_singleton_component<C>(data);
    if (old_singleton)
    {
        delete old_singleton;
    }
    {
        C *new_singleton = new C;
        data.singleton_components_[hash] = (void *)new_singleton;
        return new_singleton;
    }
}

template <typename C>
C *set_singleton_component(ComponentManagerData &data, C &&c)
{
    constexpr size_t hash = MetaType::build_info<C>().hash_code();
    C *old_singleton = get_singleton_component<C>(data);
    if (old_singleton)
    {
        *old_singleton = c;
        return old_singleton;
    }
    else
    {
        C *new_singleton = new C(c);
        data.singleton_components_[hash] = (void *)new_singleton;
        return new_singleton;
    }
}

void release_singleton_component(ComponentManagerData &data);

} // namespace Component

} // namespace cloud::world::ecs