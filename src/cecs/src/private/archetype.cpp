#include "archetype.h"
#include "chunk.h"

namespace cloud::world::ecs
{

ChunkPoolData::~ChunkPoolData()
{
    for (auto chunk : chunks)
    {
        Chunk::destroy_chunk(chunk);
    }
    chunks.clear();
}

std::pair<Chunk *, size_t>
    ChunkPool::get_or_create_chunk(ChunkPoolData &data,
                                   const std::vector<const MetaType *> &metas)
{
    Chunk *target_chunk{nullptr};
    size_t index = -1;

    auto new_chunk = [&](Chunk *&chunk) {
        chunk = Chunk::create_chunk(metas);
        data.chunks.push_back(chunk);
        return data.chunks.size() - 1;
    };

    if (data.chunks.empty())
    {
        index = new_chunk(target_chunk);
    }
    else
    {
        for (int i = 0; i < data.chunks.size(); i++)
        {
            if (!data.chunks[i]->is_full())
            {
                target_chunk = data.chunks[i];
                index = i;
                break;
            }
        }
        if (!target_chunk)
        {
            index = new_chunk(target_chunk);
        }
    }
    return {target_chunk, index};
}

void ChunkPool::destory_chunk(ChunkPoolData &data, size_t index)
{
    assert(index < data.chunks.size());
    assert(!data.chunks[index]->is_empty() && "try to delete not null chunk");
    assert(false && "not implement");
}

ArchetypeData Archetype::create_archetype_data(MaskType mask,
                                               const MetaTypeList &metas)
{
    ArchetypeData data;
    data.mask_ = mask;
    data.meta_list_ = metas;
    return data;
}

void Archetype::add_entity(ArchetypeData &data, EntityInfo &info)
{
    auto [chunk, index] =
        ChunkPool::get_or_create_chunk(data.chunk_pool_data_, data.meta_list_);
    info.index_in_chunk = chunk->insert(true);
    info.archetype_mask = data.mask_;
    info.chunk = chunk;
    info.index_in_archetype_ = index;
}

bool Archetype::destroy_entity(ArchetypeData &data, EntityInfo &info)
{
    assert(info.chunk != nullptr);
    info.chunk->erase(info.index_in_chunk);
    return true;
}

std::vector<Chunk *> &Archetype::get_chunks(ArchetypeData &data)
{
    return data.chunk_pool_data_.chunks;
}

ArchetypeManagerData::~ArchetypeManagerData()
{
    // release_archetype should be called before release this class.
    assert(archetypes.size() == 0);
}

void Archetype::release_archetypes(ArchetypeManagerData &data)
{
    for (auto [_, arch] : data.archetypes)
    {
        delete arch;
        arch = nullptr;
    }
    data.archetypes.clear();
}

ArchetypeData *Archetype::get_archetype(ArchetypeManagerData &data,
                                        MaskType mask)
{
    auto iter = data.archetypes.find(mask);
    return iter != data.archetypes.end() ? iter->second : nullptr;
}

ArchetypeData *Archetype::create_archetype(ArchetypeManagerData &data,
                                           MaskType mask,
                                           MetaTypeList meta_type_list)
{
    ArchetypeData *archetype = new ArchetypeData();
    archetype->mask_ = mask;
    archetype->meta_list_ = meta_type_list;
    data.archetypes.emplace(mask, archetype);
    return archetype;
}

void Archetype::for_each_matching_archetype(
    ArchetypeManagerData &data,
    MaskType mask,
    std::function<void(ArchetypeData *)> cb)
{
    for (auto &[type_mask, arch] : data.archetypes)
    {
        if ((type_mask & mask) == mask)
        {
            cb(arch);
        }
    }
}

bool Archetype::destroy_archetype(ArchetypeManagerData &data,
                                  ArchetypeData *archetype)
{
    if (archetype == nullptr)
        return false;

    auto iter = data.archetypes.find(archetype->mask_);
    if (iter == data.archetypes.end())
        return false;

    delete iter->second;
    data.archetypes.erase(iter);
    return true;
}

} // namespace cloud::world::ecs
