#include "entity.h"
#include <cassert>

namespace cloud::world::ecs
{
EntityInfoPool::~EntityInfoPool()
{
    for (auto ptr : entity_infos_)
    {
        delete ptr;
    }
    entity_infos_.clear();
}

EntityID EntityInfoPool::get_or_create()
{
    EntityID id{};
    if (free_list_.empty())
    {
        id.index = entity_infos_.size();
        EntityInfo *info = new EntityInfo();
        entity_infos_.push_back(info);
        info->version = 1;
        reset_info(*info);
        id.version = 1;
    }
    else
    {
        id.index = free_list_.front();
        free_list_.pop();
        EntityInfo *info = entity_infos_[id.index];
        id.version = ++info->version;
    }
    return id;
}

EntityInfo *EntityInfoPool::get(const EntityID &id)
{
    assert(id.index < entity_infos_.size());
    auto info_ptr = entity_infos_[id.index];
    return info_ptr->version == id.version ? info_ptr : nullptr;
}

const EntityInfo *EntityInfoPool::get(const EntityID &id) const
{
    assert(id.index < entity_infos_.size());
    auto info_ptr = entity_infos_[id.index];
    return info_ptr->version == id.version ? info_ptr : nullptr;
}

void EntityInfoPool::destroy(const EntityID &id)
{
    auto info_ptr = get(id);
    assert(info_ptr);
    reset_info(*info_ptr);
    free_list_.push(id.index);
}

EntityInfo *EntityInfoPool::operator[](const EntityID &id) { return get(id); }

const EntityInfo *EntityInfoPool::operator[](const EntityID &id) const
{
    return get(id);
}

void EntityInfoPool::reset_info(EntityInfo &info)
{
    info.archetype_mask = 0;
    info.chunk = nullptr;
    info.index_in_archetype_ = InvalidIndex;
    info.index_in_chunk = InvalidIndex;
}

} // namespace cloud::world::ecs
