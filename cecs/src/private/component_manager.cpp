#include "component_manager.h"
#include <cassert>

namespace cloud::world::ecs
{
ComponentManager::~ComponentManager()
{
    // singleton must be cleaned out of the manager.
    assert(singleton_components_.empty());
}

void ComponentManager::release_singleton_component()
{
    for (auto ptr : singleton_components_)
    {
        if (ptr.second)
        {
            delete ptr.second;
            ptr.second = nullptr;
        }
    }
    singleton_components_.clear();
}
} // namespace cloud::world::ecs