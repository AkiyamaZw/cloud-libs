#pragma once

#include "define.h"
#include "chunk.h"
#include <mutex>

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
    Chunk *chunk{nullptr};
    Index index_in_archetype_{InvalidIndex};
    Index index_in_chunk{InvalidIndex};
};

/**
 * @brief a entity info pool to contain entity and its EntityInfo
 *
 */
struct EntityManagementData
{
    std::vector<EntityInfo *> entity_infos_;
    std::queue<size_t> free_list_;
    std::mutex queue_mutex_;
    ~EntityManagementData();
};

namespace EntityManager
{
EntityID get_or_create(EntityManagementData &data);
EntityInfo *get(const EntityManagementData &data, const EntityID &id);
void destroy(EntityManagementData &data, const EntityID &id);
void reset(EntityInfo &info);
}; // namespace EntityManager
} // namespace cloud::world::ecs
