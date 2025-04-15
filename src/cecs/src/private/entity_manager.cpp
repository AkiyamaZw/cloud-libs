#include "entity_manager.h"
#include <cassert>

namespace cloud::world::ecs
{
namespace EntityManager
{
EntityID get_or_create(EntityManagementData &data)
{
    EntityID id{};
    std::lock_guard lock(data.queue_mutex_);
    if (data.free_list_.empty())
    {
        id.index = data.entity_infos_.size();
        EntityInfo *info = new EntityInfo();
        data.entity_infos_.push_back(info);
        info->version = 1;
        reset(*info);
        id.version = 1;
    }
    else
    {
        id.index = data.free_list_.front();
        data.free_list_.pop();
        EntityInfo *info = data.entity_infos_[id.index];
        id.version = ++info->version;
    }
    return id;
}

EntityInfo *get(const EntityManagementData &data, const EntityID &id)
{
    assert(id.index < data.entity_infos_.size());
    auto info_ptr = data.entity_infos_[id.index];
    return info_ptr->version == id.version ? info_ptr : nullptr;
}

void destroy(EntityManagementData &data, const EntityID &id)
{
    auto info_ptr = get(data, id);
    assert(info_ptr);
    reset(*info_ptr);
    data.free_list_.push(id.index);
}

void reset(EntityInfo &info)
{
    info.archetype_mask = 0;
    info.chunk = nullptr;
    info.index_in_archetype_ = InvalidIndex;
    info.index_in_chunk = InvalidIndex;
}
} // namespace EntityManager

EntityManagementData::~EntityManagementData()
{
    for (auto ptr : entity_infos_)
    {
        delete ptr;
    }
    entity_infos_.clear();
}

} // namespace cloud::world::ecs