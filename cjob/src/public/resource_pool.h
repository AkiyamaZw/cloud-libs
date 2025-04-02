#pragma once
#include "job_define.h"
#include "icounter.h"

namespace cloud::js::internal
{

template <typename T>
class ResourcePool;

template <typename T>
class IPoolableObject
{
  public:
    enum class ResourceState
    {
        RELEASED,
        USABLE
    };
    uint32_t get_index() const { return index_; };
    void set_index(uint32_t index) { index_ = index; }
    virtual void reset() = 0;
    void set_resource_state(ResourceState state) { state_ = state; };
    bool is_released() const { return state_ == ResourceState::RELEASED; }
    void mark(ResourceState state) { state_ = state; }

  protected:
    ResourcePool<T> *pool_{nullptr};
    friend class ResourcePool<T>;

  private:
    uint32_t index_;
    ResourceState state_{ResourceState::RELEASED};
};

template <typename T>
class CountablePoolableObject : public IPoolableObject<T>
{
  public:
    void reset() { ref_counter_.set_cnt(0); }
    void add_ref() { ref_counter_.add_cnt(); }
    static void sub_ref_and_try_release(T *obj)
    {
        uint32_t latest = obj->ref_counter_.sub_cnt();
        if (latest == 1)
        {
            obj->pool_->release(obj);
        }
    }
    uint32_t get_ref() const { return ref_counter_.get_cnt(); }

  protected:
    Countable ref_counter_;
};

template <typename T>
class ResourcePool
{
    static_assert(std::is_base_of<IPoolableObject<T>, T>::value,
                  "pooled object must inheriet from IPoolableObject");

  public:
    ResourcePool() = default;
    ~ResourcePool();

    T *get();
    T *at(uint32_t index);
    void release(T *t);

    std::string debug_info() const
    {
        return std::format("resource count: {}, freed: {}",
                           resource_pool_.size(),
                           free_list_.size());
    };

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
        ret->pool_ = this;
    }
    assert(ret->is_released());

    ret->mark(IPoolableObject<T>::ResourceState::USABLE);
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
    assert(!t->is_released());
    t->reset();
    free_list_.push_back(t->get_index());
    t->mark(IPoolableObject<T>::ResourceState::RELEASED);
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

} // namespace cloud::js::internal