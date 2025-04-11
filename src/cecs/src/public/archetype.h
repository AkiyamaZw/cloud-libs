#pragma once
#include "entity.h"

namespace cloud::world::ecs::internal
{
struct Chunk;
struct ChunkPool
{
    std::vector<Chunk *> chunks;
    std::vector<const MetaType *> metas;
    ChunkPool() = default;
    ~ChunkPool();

    std::pair<Chunk *, size_t>
        get_or_create_chunk(const std::vector<const MetaType *> &metas);
    void destory_chunk(size_t index);
};

class Archetype
{
  public:
    Archetype(MaskType mask, const MetaTypeList &metas);
    ~Archetype();

    MaskType get_mask() const { return mask_; }

    void add_entity(EntityInfo &info);

    bool destroy_entity(EntityInfo &info);

    std::vector<Chunk *> &get_chunks() { return chunk_pool_.chunks; };

  protected:
  private:
    MaskType mask_{};
    MetaTypeList meta_list_{};
    ChunkPool chunk_pool_{};
};

} // namespace cloud::world::ecs::internal