#include "archetype.h"
#include "component_manager.h"
#include "archetype_manager.h"
#include "chunk.h"

namespace cloud::world::ecs
{

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

template <typename... C>
inline internal::Archetype *Registry::create_archetype()
{
    auto [mask, meta_list] = component_manager_->get_info_for_archetype<C...>();
    return archetype_manager_->create_archetype(mask, meta_list);
}

template <typename C>
inline C &Registry::get_component(const EntityID &id)
{
    assert(e_info_pool_[id]->chunk != nullptr);
    return e_info_pool_.get(id)
        ->chunk->get_chunk_component<C>()[e_info_pool_[id]->index_in_chunk];
}

template <typename C>
inline bool Registry::has_component(const EntityID &id) const
{
    MaskType type(component_manager_->get<C>());
    return (e_info_pool_[id]->archetype_mask & type) == type;
}

template <typename... C>
inline internal::Archetype *Registry::get_or_create_archetype()
{
    MaskType mask;
    (..., (mask.set(component_manager_->get_or_register<C>())));
    internal::Archetype *archetype = archetype_manager_->get_archetype(mask);

    return archetype != nullptr ? archetype : create_archetype<C...>();
}

template <typename C>
inline C *Registry::get_singleton_component()
{
    return component_manager_->get_singleton_component<C>();
}

template <typename C>
inline C *Registry::set_singleton_component()
{
    return component_manager_->set_singleton_component<C>();
}

template <typename C>
inline C *Registry::set_singleton_component(C &&singleton)
{
    return component_manager_->set_singleton_component<C>(
        std::forward<C>(singleton));
}
} // namespace cloud::world::ecs