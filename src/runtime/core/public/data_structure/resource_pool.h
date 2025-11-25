#pragma once
#include <cassert>
#include <cstdint>

namespace cloud
{
class ResourcePool
{
public:
    static constexpr uint32_t INVALID_NUM = 0xffffffff;
    ResourcePool(uint32_t pool_size, uint32_t resource_size);
    virtual ~ResourcePool();
    void Shutdown();
    uint32_t FetchResource();
    void ReleaseResource(uint32_t index);
    void ReleaseAllResources();
    void* Access(uint32_t index);
    const void* Access(uint32_t index) const;

protected:
    uint8_t *memory_{nullptr};
    uint32_t* free_indices_{nullptr};
    uint32_t free_indices_head_{0};
    uint32_t pool_size_{16};
    uint32_t resource_size_{4};
    uint32_t used_indices_{16};
};

template<typename T>
struct TypedResourcePool: public ResourcePool
{
    TypedResourcePool(uint32_t pool_size);
    ~TypedResourcePool() override;

    T* Fetch();
    void Release(T* resource);

    T* Get(uint32_t index);
    const T* Get(uint32_t index) const;
};

template <typename T>
TypedResourcePool<T>::TypedResourcePool(uint32_t pool_size)
    :ResourcePool(pool_size, sizeof(T))
{
}

template <typename T>
TypedResourcePool<T>::~TypedResourcePool()
{
    assert(free_indices_head_ == 0);
}

template <typename T>
T * TypedResourcePool<T>::Fetch()
{
    uint32_t res_index = ResourcePool::FetchResource();
    if (res_index == INVALID_NUM)
        return nullptr;
    T* res = Get(res_index);
    // make sure pool_index in resource class
    res->pool_index = res_index;
    return res;
}

template <typename T>
void TypedResourcePool<T>::Release(T *resource)
{
    ResourcePool::ReleaseAllResources(resource->pool_index);
}

template <typename T>
T * TypedResourcePool<T>::Get(uint32_t index)
{
    return static_cast<T *>(ResourcePool::Access(index));
}

template <typename T>
const T * TypedResourcePool<T>::Get(uint32_t index) const
{
    return static_cast<const T *>(ResourcePool::Access(index));
}


}