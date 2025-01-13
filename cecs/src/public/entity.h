#pragma once

#include "define.h"

namespace cloud::world::ecs
{

namespace internal
{
struct Chunk;
}

/**
 * @brief entity id include its index and version, which marks the life
 * generation of the entity.
 *
 */
struct EntityID
{
    EntityIndex index;
    EntityVersion version;
};

/**
 * @brief records the entity base info, include version, archetype, cached chunk
 * address, and inner index
 *
 */
struct EntityInfo
{
    EntityVersion version{0};
    MaskType archetype_mask{0};
    internal::Chunk *chunk{nullptr};
    Index index_in_archetype_{InvalidIndex};
    Index index_in_chunk{InvalidIndex};
};

/**
 * @brief a entity info pool to contain entity and its EntityInfo
 *
 */
class EntityInfoPool final
{
  public:
    EntityInfoPool() = default;

    ~EntityInfoPool();

    EntityID get_or_create();

    EntityInfo *get(const EntityID &id);

    const EntityInfo *get(const EntityID &id) const;

    void destroy(const EntityID &id);

    EntityInfo *operator[](const EntityID &id);

    const EntityInfo *operator[](const EntityID &id) const;

  protected:
    void reset_info(EntityInfo &info);

  private:
    std::vector<EntityInfo *> entity_infos_;

    std::queue<size_t> free_list_;
};

} // namespace cloud::world::ecs
