#pragma once
#include "entity.h"

namespace cloud::world::ecs
{
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