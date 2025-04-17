#include "registry.h"
#include "component_manager.h"

namespace cloud::world::ecs
{

// Registry::Registry() {}

// Registry::~Registry()
// {
//     Component::release_singleton_component(component_data_);
//     Archetype::release_archetypes(archetype_data_);
// }

// bool Registry::destroy_entity(const EntityID &id)
// {
//     auto info = EntityManager::get(entity_data_, id);
//     assert(dellocate_entity(*info));
//     EntityManager::destroy(entity_data_, id);
//     return true;
// }

// void Registry::destroy_entities(const std::vector<EntityID> &ids)
// {
//     for (const auto &id : ids)
//     {
//         destroy_entity(id);
//     }
// }

// bool Registry::destroy_archetype(ArchetypeData *archetype)
// {
//     return Archetype::destroy_archetype(archetype_data_,
//     archetype);
// }

// size_t Registry::get_archetype_count() const
// {
//     return archetype_data_.archetypes.size();
// }

// EntityID Registry::allocate_entity(ArchetypeData *arch)
// {
//     EntityID id = EntityManager::get_or_create(entity_data_);
//     Archetype::add_entity(*arch,
//                                     *EntityManager::get(entity_data_, id));
//     return id;
// }

// bool Registry::dellocate_entity(EntityInfo &info)
// {
//     auto arch = Archetype::get_archetype(archetype_data_,
//                                                    info.archetype_mask);
//     return arch ? Archetype::destroy_entity(*arch, info) : false;
// }

// void Registry::for_each_matching_archetype(
//     MaskType mask, std::function<void(ArchetypeData *)> cb)
// {
//     Archetype::for_each_matching_archetype(archetype_data_, mask,
//     cb);
// }

RegistryData::~RegistryData()
{
    Component::release_singleton_component(component_data_);
    Archetype::release_archetypes(archetype_data_);
}

bool Registry::destroy_entity(RegistryData &data, const EntityID &id)
{
    auto info = EntityManager::get(data.entity_data_, id);
    assert(dellocate_entity(*info));
    EntityManager::destroy(data.entity_data_, id);
    return true;
}

void Registry::destroy_entities(RegistryData &data,
                                const std::vector<EntityID> &ids)
{
    for (const auto &id : ids)
    {
        destroy_entity(data, id);
    }
}

bool Registry::destroy_archetype(RegistryData &data, ArchetypeData *archetype)
{
    return Archetype::destroy_archetype(data.archetype_data_, archetype);
}

void Registry::for_each_matching_archetype(
    RegistryData &data, MaskType mask, std::function<void(ArchetypeData *)> cb)
{
    Archetype::for_each_matching_archetype(data.archetype_data_, mask, cb);
}

} // namespace cloud::world::ecs
