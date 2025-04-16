#pragma once
#include <functional>
#include "entity.h"
#include "entity_manager.h"
#include "component_manager.h"
#include "archetype.h"

namespace cloud::world::ecs
{
namespace internal
{
struct ArchetypeData;
};

class ComponentManager;

class Registry final
{
    friend class TestWorld;
    friend struct Selector;

  public:
    Registry();

    ~Registry();

    Registry(const Registry &) = delete;
    Registry(Registry &&) = delete;
    Registry &operator=(const Registry &) = delete;
    Registry &operator=(Registry &&) = delete;

    // ↓ ↓ ↓ entity ↓ ↓ ↓
    template <typename... C>
    EntityID create_entity();

    template <typename... C>
    std::vector<EntityID> create_entities(size_t size);

    bool destroy_entity(const EntityID &id);

    void destroy_entities(const std::vector<EntityID> &ids);
    // ↑ ↑ ↑ entity ↑ ↑ ↑

    //  ↓ ↓ ↓ component ↓ ↓ ↓
    template <typename C>
    C &get_component(const EntityID &id);

    template <typename C>
    bool has_component(const EntityID &id) const;

    template <typename C>
    C *get_singleton_component();

    template <typename C>
    C *set_singleton_component();

    template <typename C>
    C *set_singleton_component(C &&singleton);
    // ↑ ↑ ↑ component ↑ ↑ ↑

  protected:
    // ↓ ↓ ↓ archetype ↓ ↓ ↓

    template <typename... C>
    internal::ArchetypeData *get_or_create_archetype();

    bool destroy_archetype(internal::ArchetypeData *archetype);

    size_t get_archetype_count() const;
    // ↑ ↑ ↑ archetype ↑ ↑ ↑

  protected:
    EntityID allocate_entity(internal::ArchetypeData *arch);
    bool dellocate_entity(EntityInfo &info);
    internal::ArchetypeData *get_archetype(MaskType mask);
    void for_each_matching_archetype(
        MaskType mask, std::function<void(internal::ArchetypeData *)> cb);

  public:
    EntityManagementData entity_data_;
    internal::ArchetypeManagerData archetype_data_;
    ComponentManagerData component_data_;
};

struct RegistryData
{
    ~RegistryData();
    EntityManagementData entity_data_;
    internal::ArchetypeManagerData archetype_data_;
    ComponentManagerData component_data_;
};

namespace RegistryEx
{
template <typename... C>
EntityID create_entity(RegistryData &data);

template <typename... C>
internal::ArchetypeData *get_or_create_archetype(RegistryData &data);
} // namespace RegistryEx

} // namespace cloud::world::ecs
#include "registry.inc"