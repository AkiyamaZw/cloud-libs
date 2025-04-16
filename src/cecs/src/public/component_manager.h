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
class ComponentManager final
{
  public:
    static constexpr uint64_t INVALID_INDEX_ID =
        std::numeric_limits<uint64_t>::max();

  public:
    ComponentManager() = default;
    ~ComponentManager();

    ComponentManager(const ComponentManager &) = delete;
    ComponentManager(ComponentManager &&) = delete;
    ComponentManager &operator=(const ComponentManager &) = delete;
    ComponentManager &operator=(ComponentManager &&) = delete;

    // release singleton component
    void release_singleton_component();

    // get a unique id for component
    template <typename T>
    size_t get() const;

    // get or register a unique id for component
    template <typename T>
    size_t get_or_register();

    // get component T metatype
    template <typename T>
    const MetaType *get_metatype();

    // get components metatype lists
    template <typename... C>
    MetaTypeList get_metatypes();

    template <typename... C>
    MetaTypeList get_info_for_archetype(MaskType &mask);

    template <typename C>
    C *get_singleton_component();

    template <typename C>
    C *set_singleton_component();

    template <typename C>
    C *set_singleton_component(C &&c);

  private:
    // a component unique id generator
    uint64_t next_index_{1};

    // hash to unique id mapping
    std::unordered_map<size_t, uint64_t> type_to_index_{};

    // hash to meta type mapping
    std::unordered_map<uint64_t, MetaType> metatype_map_{};

    // container for singleton component
    std::unordered_map<size_t, void *> singleton_components_{};
};

template <typename T>
inline size_t ComponentManager::get() const
{

    auto it = type_to_index_.find(typeid(T).hash_code());
    if (it != type_to_index_.end())
    {
        return it->second;
    }
    return INVALID_INDEX_ID;
}

template <typename T>
inline size_t ComponentManager::get_or_register()
{
    auto index = get<T>();
    if (index != -1)
    {
        return index;
    }
    size_t new_index = next_index_++;
    type_to_index_[typeid(T).hash_code()] = new_index;
    return new_index;
}

template <typename T>
inline const MetaType *ComponentManager::get_metatype()
{
    const auto &info = MetaType::build_info<T>();
    size_t hash = info.hash_code();
    auto type = metatype_map_.find(hash);
    if (type == metatype_map_.end())
    {
        metatype_map_.emplace(hash, MetaType::build<T>());
    }
    return &metatype_map_[hash];
}

template <typename... C>
inline MetaTypeList ComponentManager::get_metatypes()
{
    const MetaType *types[] = {get_metatype<C>()...};
    size_t num = (sizeof(types) / sizeof(*types));
    return {types, types + num};
}

template <typename... C>
inline MetaTypeList ComponentManager::get_info_for_archetype(MaskType &mask)
{
    (..., (mask.set(get_or_register<C>())));

    return get_metatypes<C...>();
}

template <typename C>
inline C *ComponentManager::get_singleton_component()
{
    constexpr size_t hash = MetaType::build_info<C>().hash_code();
    auto lookup = singleton_components_.find(hash);

    return lookup != singleton_components_.end() ? (C *)(lookup->second)
                                                 : nullptr;
}

template <typename C>
inline C *ComponentManager::set_singleton_component()
{
    constexpr size_t hash = MetaType::build_info<C>().hash_code();
    C *old_singleton = get_singleton_component<C>();
    if (old_singleton)
    {
        delete old_singleton;
    }
    {
        C *new_singleton = new C;
        singleton_components_[hash] = (void *)new_singleton;
        return new_singleton;
    }
}
template <typename C>
inline C *ComponentManager::set_singleton_component(C &&c)
{
    constexpr size_t hash = MetaType::build_info<C>().hash_code();
    C *old_singleton = get_singleton_component<C>();
    if (old_singleton)
    {
        *old_singleton = c;
        return old_singleton;
    }
    else
    {
        C *new_singleton = new C(c);
        singleton_components_[hash] = (void *)new_singleton;
        return new_singleton;
    }
}
} // namespace cloud::world::ecs