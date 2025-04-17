#pragma once
#include "memory.h"
#include "meta_type.h"
#include "bitbool.h"
#include <span>

namespace cloud::world::ecs
{

struct SlotPool
{
    SlotPool() = default;

    ~SlotPool();

    size_t get();

    void give_back(size_t index);

    size_t get_size() const { return allocated.size() - free_list.size(); }

    size_t get_allocated_size() const { return allocated.size(); }

    std::vector<size_t> allocated;
    std::queue<size_t> free_list;
};

struct Chunk
{
    struct ChunkComponentMeta
    {
        // define the Component meta data
        const MetaType *meta;
        // define the Component array start address
        uint32_t offset_in_block;
        void *ptr{nullptr};
    };

    // raw data block in memory
    Block *block{nullptr};
    // the number of each component instance
    uint32_t capacity{0};
    // slot pool
    SlotPool slot_pool;
    // a uint64_t based alive flags array
    BitBool64 *entity_live_flags{nullptr};

    // hash to ChunkComponentMeta
    std::unordered_map<size_t, ChunkComponentMeta> hash_mapping;

    Chunk() = default;
    ~Chunk();

    uint32_t get_type_count() const { return hash_mapping.size(); }

    uint32_t get_capacity() const { return capacity; }

    // this size is the alive count
    uint32_t get_size() const { return slot_pool.get_size(); }

    // this size is the max pool size in now;
    uint32_t get_allocated_size() const
    {
        return slot_pool.get_allocated_size();
    }

    bool is_empty() const { return get_size() == 0; }

    bool is_full() const { return get_size() == capacity; }

    // build the component meta
    void build_component_list(const std::vector<const MetaType *> &meta);

    // factory function
    static Chunk *create_chunk(const std::vector<const MetaType *> &meta);

    // factory function
    static void destroy_chunk(Chunk *chunk);

    // get the array of T
    template <typename T>
    auto get_chunk_component()
    {
        using TT = std::remove_reference_t<T>;

        size_t hash = MetaType::build_info<T>().hash_code();
        auto iter = hash_mapping.find(hash);
        if (iter == hash_mapping.end())
        {
            return std::span<TT>{};
        }
        return std::span<TT>{(TT *)iter->second.ptr, get_allocated_size()};
    }

    std::vector<ChunkComponentMeta *>
        locate_components(const MetaTypeList &metas);

    // insert a record into chunk
    size_t insert(bool initialze = true);

    // 'destroy' a record from chunk
    void erase(size_t index);

    bool get_alive(size_t index) const;
};

} // namespace cloud::world::ecs