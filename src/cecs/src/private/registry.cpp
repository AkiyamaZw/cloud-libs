#include "registry.h"
#include "component_manager.h"
#include "archetype_manager.h"

namespace cloud::world::ecs
{
Registry::Registry()
{
    component_manager_ = std::make_unique<ComponentManager>();
    archetype_manager_ = std::make_unique<ArchetypeManager>();
}

Registry::~Registry()
{
    component_manager_->release_singleton_component();
    archetype_manager_->release_archetype();
}

bool Registry::destroy_entity(const EntityID &id)
{
    auto info = e_info_pool_[id];
    assert(dellocate_entity(*e_info_pool_[id]));
    e_info_pool_.destroy(id);
    return true;
}

void Registry::destroy_entities(const std::vector<EntityID> &ids)
{
    for (const auto &id : ids)
    {
        destroy_entity(id);
    }
}

bool Registry::destroy_archetype(internal::Archetype *archetype)
{
    return archetype_manager_->destroy_archetype(archetype);
}

size_t Registry::get_archetype_count() const
{
    return archetype_manager_->get_archetype_count();
}

EntityID Registry::allocate_entity(internal::Archetype *arch)
{
    EntityID id = e_info_pool_.get_or_create();
    arch->add_entity(*e_info_pool_[id]);
    return id;
}

bool Registry::dellocate_entity(EntityInfo &info)
{
    auto arch = get_archetype(info.archetype_mask);
    return arch ? arch->destroy_entity(info) : false;
}

internal::Archetype *Registry::get_archetype(MaskType mask)
{
    return archetype_manager_->get_archetype(mask);
}

void Registry::for_each_matching_archetype(
    MaskType mask, std::function<void(internal::Archetype *)> cb)
{
    archetype_manager_->for_each_matching_archetype(mask, cb);
}

} // namespace cloud::world::ecs
