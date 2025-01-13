#include "archetype_manager.h"
#include "archetype.h"

namespace cloud::world::ecs
{
ArchetypeManager::~ArchetypeManager()
{
    // release_archetype should be called before release this class.
    assert(archetypes_.size() == 0);
}

void ArchetypeManager::release_archetype()
{
    for (auto [_, arch] : archetypes_)
    {
        delete arch;
        arch = nullptr;
    }
    archetypes_.clear();
}

internal::Archetype *ArchetypeManager::get_archetype(MaskType mask)
{
    auto iter = archetypes_.find(mask);
    return iter != archetypes_.end() ? iter->second : nullptr;
}

internal::Archetype *
    ArchetypeManager::create_archetype(MaskType mask,
                                       MetaTypeList meta_type_list)
{
    internal::Archetype *archetype =
        new internal::Archetype(mask, meta_type_list);
    archetypes_[mask] = archetype;
    return archetype;
}

void ArchetypeManager::for_each_matching_archetype(
    MaskType mask, std::function<void(internal::Archetype *)> cb)
{
    for (auto &[type_mask, arch] : archetypes_)
    {
        if ((type_mask & mask) == mask)
        {
            cb(arch);
        }
    }
}

bool ArchetypeManager::destroy_archetype(internal::Archetype *archetype)
{
    if (archetype == nullptr)
        return false;

    auto iter = archetypes_.find(archetype->get_mask());
    if (iter == archetypes_.end())
        return false;

    delete iter->second;
    archetypes_.erase(iter);
    return true;
}

size_t ArchetypeManager::get_archetype_count() const
{
    return archetypes_.size();
}
} // namespace cloud::world::ecs