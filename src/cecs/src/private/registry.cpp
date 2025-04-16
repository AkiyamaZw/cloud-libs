#include "registry.h"
#include "component_manager.h"

namespace cloud::world::ecs
{
Registry::Registry() {}

Registry::~Registry()
{
    Component::release_singleton_component(component_data_);
    internal::Archetype::release_archetypes(archetype_data_);
}

bool Registry::destroy_entity(const EntityID &id)
{
    auto info = EntityManager::get(entity_data_, id);
    assert(dellocate_entity(*info));
    EntityManager::destroy(entity_data_, id);
    return true;
}

void Registry::destroy_entities(const std::vector<EntityID> &ids)
{
    for (const auto &id : ids)
    {
        destroy_entity(id);
    }
}

bool Registry::destroy_archetype(internal::ArchetypeData *archetype)
{
    return internal::Archetype::destroy_archetype(archetype_data_, archetype);
}

size_t Registry::get_archetype_count() const
{
    return archetype_data_.archetypes.size();
}

EntityID Registry::allocate_entity(internal::ArchetypeData *arch)
{
    EntityID id = EntityManager::get_or_create(entity_data_);
    internal::Archetype::add_entity(*arch,
                                    *EntityManager::get(entity_data_, id));
    return id;
}

bool Registry::dellocate_entity(EntityInfo &info)
{
    auto arch = internal::Archetype::get_archetype(archetype_data_,
                                                   info.archetype_mask);
    return arch ? internal::Archetype::destroy_entity(*arch, info) : false;
}

void Registry::for_each_matching_archetype(
    MaskType mask, std::function<void(internal::ArchetypeData *)> cb)
{
    internal::Archetype::for_each_matching_archetype(archetype_data_, mask, cb);
}

RegistryData::~RegistryData()
{
    Component::release_singleton_component(component_data_);
    internal::Archetype::release_archetypes(archetype_data_);
}

} // namespace cloud::world::ecs
