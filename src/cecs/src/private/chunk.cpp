#include "chunk.h"

namespace cloud::world::ecs
{
SlotPool::~SlotPool() { allocated.clear(); }

size_t SlotPool::get()
{
    size_t new_index = -1;
    if (free_list.empty())
    {
        new_index = allocated.size();
        allocated.push_back(new_index);
    }
    else
    {
        new_index = free_list.front();
        free_list.pop();
    }
    return new_index;
}

void SlotPool::give_back(size_t index) { free_list.push(index); }

Chunk::~Chunk()
{
    if (entity_live_flags)
    {
        delete entity_live_flags;
    }
}

void Chunk::build_component_list(const std::vector<const MetaType *> &metas)
{
    size_t compnent_size = 0;
    // 1. one bit alive flag
    compnent_size += 1;
    // 2. each comp
    for (const auto meta : metas)
    {
        compnent_size += meta->size;
    }

    capacity = sizeof(Block::storage) / compnent_size - 2;

    uint32_t offset = 0;

    // using BitBool64 for entity alive flags.
    offset += ((capacity + 63) / 64) * 8;

    for (const auto &meta : metas)
    {
        bool is_zero_size = meta->align == 0;
        if (!is_zero_size)
        {
            // add align size
            size_t remainder = offset % meta->align;
            offset += (meta->align - remainder);
        }

        auto [_, succ] = hash_mapping.insert(
            {meta->hash, {meta, offset, (void *)((byte *)block + offset)}});
        assert(succ);
        if (!is_zero_size)
        {
            // add comp size
            offset += meta->size * capacity;
        }
    }
    assert(offset <= sizeof(Block::storage));
}

Chunk *Chunk::create_chunk(const std::vector<const MetaType *> &meta)
{
    Chunk *chunk = new Chunk();
    chunk->block = new Block();
    chunk->build_component_list(meta);
    // this must be set in final.
    chunk->entity_live_flags =
        new BitBool64((void *)chunk->block, chunk->capacity);
    return chunk;
}

void Chunk::destroy_chunk(Chunk *chunk)
{
    if (chunk->block->header)
        delete chunk->block->header;
    delete chunk->block;
    delete chunk;
    chunk = nullptr;
}

std::vector<Chunk::ChunkComponentMeta *>
    Chunk::locate_components(const MetaTypeList &metas)
{
    std::vector<ChunkComponentMeta *> res;
    for (auto meta : metas)
    {
        auto iter = hash_mapping.find(meta->hash);
        assert(iter != hash_mapping.end());
        res.push_back(&iter->second);
    }
    return res;
}

size_t Chunk::insert(bool initialze)
{
    assert(get_size() < capacity);
    size_t index = slot_pool.get();

    if (initialze)
    {
        for (const auto &[_, meta] : hash_mapping)
        {
            const MetaType *type = meta.meta;
            void *ptr = (void *)((byte *)meta.ptr + (type->size * index));
            type->constructor(ptr);
        }
    }
    entity_live_flags->set_bit(index);
    return index;
}

void Chunk::erase(size_t index)
{
    assert(index < get_allocated_size());

    slot_pool.give_back(index);

    for (const auto &[_, meta] : hash_mapping)
    {
        const MetaType *type = meta.meta;
        void *ptr = (void *)((byte *)meta.ptr + (type->size * index));
        type->destructor(ptr);
    }

    entity_live_flags->clear_bit(index);
}

bool Chunk::get_alive(size_t index) const
{
    return entity_live_flags->get_bit(index);
}
} // namespace cloud::world::ecs