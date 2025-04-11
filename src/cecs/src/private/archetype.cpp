#include "archetype.h"
#include "chunk.h"

namespace cloud::world::ecs::internal
{

ChunkPool::~ChunkPool()
{
    for (auto chunk : chunks)
    {
        Chunk::destroy_chunk(chunk);
    }
    chunks.clear();
}

std::pair<Chunk *, size_t>
    ChunkPool::get_or_create_chunk(const std::vector<const MetaType *> &metas)
{
    Chunk *target_chunk{nullptr};
    size_t index = -1;

    auto new_chunk = [&](Chunk *&chunk) {
        chunk = Chunk::create_chunk(metas);
        chunks.push_back(chunk);
        return chunks.size() - 1;
    };

    if (chunks.empty())
    {
        index = new_chunk(target_chunk);
    }
    else
    {
        for (int i = 0; i < chunks.size(); i++)
        {
            if (!chunks[i]->is_full())
            {
                target_chunk = chunks[i];
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

void ChunkPool::destory_chunk(size_t index)
{
    assert(index < chunks.size());
    assert(!chunks[index]->is_empty() && "try to delete not null chunk");
    assert(false && "not implement");
}

Archetype::Archetype(MaskType mask, const MetaTypeList &metas)
    : mask_(mask)
    , meta_list_(metas)
{
}

Archetype::~Archetype() {}

void Archetype::add_entity(EntityInfo &info)
{
    auto [chunk, index] = chunk_pool_.get_or_create_chunk(meta_list_);
    info.index_in_chunk = chunk->insert(true);
    info.archetype_mask = mask_;
    info.chunk = chunk;
    info.index_in_archetype_ = index;
}

bool Archetype::destroy_entity(EntityInfo &info)
{
    assert(info.chunk != nullptr);
    info.chunk->erase(info.index_in_chunk);
    return true;
}

} // namespace cloud::world::ecs::internal