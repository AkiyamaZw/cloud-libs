#pragma once
#include "entity.h"
#include <functional>

namespace cloud::world::ecs::internal
{
struct Chunk;

struct ChunkPoolData
{
    ~ChunkPoolData();
    std::vector<Chunk *> chunks;
    std::vector<const MetaType *> metas;
};

namespace ChunkPool
{
std::pair<Chunk *, size_t>
    get_or_create_chunk(ChunkPoolData &data,
                        const std::vector<const MetaType *> &metas);
void destory_chunk(ChunkPoolData &data, size_t index);
} // namespace ChunkPool

struct ArchetypeData
{
    MaskType mask_{};
    MetaTypeList meta_list_{};
    ChunkPoolData chunk_pool_data_{};
};

struct ArchetypeManagerData
{
    ~ArchetypeManagerData();
    std::unordered_map<MaskType, ArchetypeData *> archetypes;
};

namespace Archetype
{
ArchetypeData create_archetype_data(MaskType mask, const MetaTypeList &metas);
void add_entity(ArchetypeData &data, EntityInfo &info);
bool destroy_entity(ArchetypeData &data, EntityInfo &info);
std::vector<Chunk *> &get_chunks(ArchetypeData &data);

/* called when system shutdown */
void release_archetypes(ArchetypeManagerData &data);
/*  get an archetype */
ArchetypeData *get_archetype(ArchetypeManagerData &data, MaskType mask);
/* create and archetype */
ArchetypeData *create_archetype(ArchetypeManagerData &data,
                                MaskType mask,
                                MetaTypeList meta_type_list);
/* each archetype operator*/
void for_each_matching_archetype(ArchetypeManagerData &data,
                                 MaskType mask,
                                 std::function<void(ArchetypeData *)> cb);
bool destroy_archetype(ArchetypeManagerData &data, ArchetypeData *archetype);
}; // namespace Archetype

} // namespace cloud::world::ecs::internal