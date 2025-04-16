#include "chunk.h"

namespace cloud::world::ecs
{
namespace RegistryEx
{
template <typename... C>
EntityID create_entity(RegistryData &data)
{
    auto archetype = get_or_create_archetype<C...>(data);
    EntityID id = EntityManager::get_or_create(data.entity_data_);
    internal::Archetype::add_entity(*archetype,
                                    *EntityManager::get(data.entity_data_, id));
    return id;
}

template <typename... C>
internal::ArchetypeData *get_or_create_archetype(RegistryData &data)
{
    MaskType mask;
    (..., (mask.set(Component::get_or_register<C>(data.component_data_))));
    internal::ArchetypeData *archetype =
        internal::Archetype::get_archetype(data.archetype_data_, mask);
    if (archetype != nullptr)
    {
        return archetype;
    }
    auto meta_list =
        Component::get_info_for_archetype<C...>(data.component_data_, mask);
    return internal::Archetype::create_archetype(
        data.archetype_data_, mask, meta_list);
}
} // namespace RegistryEx

template <typename... C>
EntityID Registry::create_entity()
{
    return allocate_entity(get_or_create_archetype<C...>());
}

template <typename... C>
inline std::vector<EntityID> Registry::create_entities(size_t size)
{
    std::vector<EntityID> res;
    for (size_t x = 0; x < size; ++x)
    {
        res.push_back(create_entity<C...>());
    }
    return res;
}

template <typename C>
inline C &Registry::get_component(const EntityID &id)
{
    EntityInfo *info = EntityManager::get(entity_data_, id);
    assert(info != nullptr && info->chunk != nullptr);
    return info->chunk->get_chunk_component<C>()[info->index_in_chunk];
}

template <typename C>
inline bool Registry::has_component(const EntityID &id) const
{
    MaskType type(Component::get_id<C>(component_data_));
    return (EntityManager::get(entity_data_, id)->archetype_mask & type) ==
           type;
}

template <typename... C>
inline internal::ArchetypeData *Registry::get_or_create_archetype()
{
    MaskType mask;
    (..., (mask.set(Component::get_or_register<C>(component_data_))));
    internal::ArchetypeData *archetype =
        internal::Archetype::get_archetype(archetype_data_, mask);
    if (archetype != nullptr)
    {
        return archetype;
    }
    auto meta_list =
        Component::get_info_for_archetype<C...>(component_data_, mask);
    return internal::Archetype::create_archetype(
        archetype_data_, mask, meta_list);
}

template <typename C>
inline C *Registry::get_singleton_component()
{
    return Component::get_singleton_component<C>(component_data_);
}

template <typename C>
inline C *Registry::set_singleton_component()
{
    return Component::set_singleton_component<C>(component_data_);
}

template <typename C>
inline C *Registry::set_singleton_component(C &&singleton)
{
    return Component::set_singleton_component<C>(component_data_,
                                                 std::forward<C>(singleton));
}
} // namespace cloud::world::ecs