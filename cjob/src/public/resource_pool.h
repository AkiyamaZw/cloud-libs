#pragma once
#include "job_define.h"

namespace cloud::internal
{

class IPoolableObject
{
  public:
    uint32_t get_index() const { return index_; };
    void set_index(uint32_t index) { index_ = index; }
    virtual void reset() = 0;

  private:
    uint32_t index_;
};

template <typename T>
class ResourcePool
{
    static_assert(std::is_base_of<IPoolableObject, T>::value,
                  "pooled object must inheriet from IPoolableObject");

  public:
    ResourcePool() = default;
    ~ResourcePool();

    T *get();
    T *at(uint32_t index);
    void release(T *t);

  private:
    std::mutex mutex_;
    std::vector<T *> resource_pool_;
    std::deque<uint32_t> free_list_;
};

// ↓ ↓ ↓ implement ↓ ↓ ↓
template <typename T>
inline T *ResourcePool<T>::get()
{
    T *ret = nullptr;
    std::lock_guard lock(mutex_);
    if (!free_list_.empty())
    {
        uint32_t index = free_list_.front();
        free_list_.pop_front();
        ret = resource_pool_[index];
    }
    else
    {
        ret = new T();
        ret->set_index(resource_pool_.size());
        resource_pool_.push_back(ret);
    }

    return ret;
}

template <typename T>
inline T *ResourcePool<T>::at(uint32_t index)
{
    std::lock_guard lock(mutex_);
    assert(index < resource_pool_.size());
    return resource_pool_[index];
}

template <typename T>
inline void ResourcePool<T>::release(T *t)
{
    std::lock_guard lock(mutex_);
    assert(t != nullptr);
    t->reset();
    free_list_.push_back(t->get_index());
}

template <typename T>
inline ResourcePool<T>::~ResourcePool()
{
    std::lock_guard lock(mutex_);
    for (auto ptr : resource_pool_)
    {
        delete ptr;
        ptr = nullptr;
    }
    resource_pool_.clear();
    free_list_.clear();
}

// ↑ ↑ ↑ implement ↑ ↑ ↑

} // namespace cloud::internal