#pragma once
#include <functional>
#include "entity.h"
#include "component_manager.h"
#include "archetype.h"

namespace cloud::world::ecs
{
struct RegistryData
{
    ~RegistryData();
    EntityManagementData entity_data_;
    ArchetypeManagerData archetype_data_;
    ComponentManagerData component_data_;
};

namespace Registry
{
template <typename... C>
EntityID create_entity(RegistryData &data);

template <typename... C>
std::vector<EntityID> create_entities(RegistryData &data, size_t size);

bool destroy_entity(RegistryData &data, const EntityID &id);

void destroy_entities(RegistryData &data, const std::vector<EntityID> &ids);

template <typename... C>
ArchetypeData *get_or_create_archetype(RegistryData &data);

bool destroy_archetype(RegistryData &data, ArchetypeData *archetype);

template <typename C>
C &get_component(RegistryData &data, const EntityID &id);

template <typename C>
bool has_component(const RegistryData &data, const EntityID &id);

template <typename C>
C *get_singleton_component(RegistryData &data);

template <typename C>
C *set_singleton_component(RegistryData &data);

template <typename C>
C *set_singleton_component(RegistryData &data, C &&singleton);

void for_each_matching_archetype(RegistryData &data,
                                 MaskType mask,
                                 std::function<void(ArchetypeData *)> cb);
} // namespace Registry

} // namespace cloud::world::ecs
#include "registry.inc"