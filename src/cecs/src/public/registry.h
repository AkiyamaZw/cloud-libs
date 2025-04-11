#pragma once
#include <functional>
#include "entity.h"

namespace cloud::world::ecs
{
namespace internal
{
class Archetype;
};

class ComponentManager;
class ArchetypeManager;

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
    internal::Archetype *get_or_create_archetype();

    template <typename... C>
    internal::Archetype *create_archetype();

    bool destroy_archetype(internal::Archetype *archetype);

    size_t get_archetype_count() const;
    // ↑ ↑ ↑ archetype ↑ ↑ ↑

  protected:
    EntityID allocate_entity(internal::Archetype *arch);
    bool dellocate_entity(EntityInfo &info);
    internal::Archetype *get_archetype(MaskType mask);
    void for_each_matching_archetype(
        MaskType mask, std::function<void(internal::Archetype *)> cb);

  private:
    EntityInfoPool e_info_pool_;
    std::unique_ptr<ComponentManager> component_manager_;
    std::unique_ptr<ArchetypeManager> archetype_manager_;
};

// template <typename T>
// void op(EntityInfo &info, T &comp)
// {

//     id + meta_pool-- > chunk-- > index_in_chunk
// }
} // namespace cloud::world::ecs
#include "registry.inc"