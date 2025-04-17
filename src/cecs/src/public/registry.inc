#include "chunk.h"

namespace cloud::world::ecs
{
namespace Registry
{
template <typename... C>
EntityID create_entity(RegistryData &data)
{
    auto archetype = get_or_create_archetype<C...>(data);
    EntityID id = EntityManager::get_or_create(data.entity_data_);
    Archetype::add_entity(*archetype,
                          *EntityManager::get(data.entity_data_, id));
    return id;
}

template <typename... C>
std::vector<EntityID> create_entities(RegistryData &data, size_t size)
{
    std::vector<EntityID> res;
    for (size_t x = 0; x < size; ++x)
    {
        res.push_back(create_entity<C...>(data));
    }
    return res;
}

template <typename... C>
ArchetypeData *get_or_create_archetype(RegistryData &data)
{
    MaskType mask;
    (..., (mask.set(Component::get_or_register<C>(data.component_data_))));
    ArchetypeData *archetype =
        Archetype::get_archetype(data.archetype_data_, mask);
    if (archetype != nullptr)
    {
        return archetype;
    }
    auto meta_list =
        Component::get_info_for_archetype<C...>(data.component_data_, mask);
    return Archetype::create_archetype(data.archetype_data_, mask, meta_list);
}

template <typename C>
C &get_component(RegistryData &data, const EntityID &id)
{
    EntityInfo *info = EntityManager::get(data.entity_data_, id);
    assert(info != nullptr && info->chunk != nullptr);
    return info->chunk->get_chunk_component<C>()[info->index_in_chunk];
}

template <typename C>
bool has_component(const RegistryData &data, const EntityID &id)
{
    MaskType type(Component::get_id<C>(data.component_data_));
    return (EntityManager::get(data.entity_data_, id)->archetype_mask & type) ==
           type;
}

template <typename C>
C *get_singleton_component(RegistryData &data)
{
    return Component::get_singleton_component<C>(data.component_data_);
}

template <typename C>
C *set_singleton_component(RegistryData &data)
{
    return Component::set_singleton_component<C>(data.component_data_);
}

template <typename C>
C *set_singleton_component(RegistryData &data, C &&singleton)
{
    return Component::set_singleton_component<C>(data.component_data_,
                                                 std::forward<C>(singleton));
}
} // namespace Registry
} // namespace cloud::world::ecs