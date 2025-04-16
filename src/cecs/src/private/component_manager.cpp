#include "component_manager.h"
#include <cassert>

namespace cloud::world::ecs
{

ComponentManagerData::~ComponentManagerData()
{
    // singleton must be cleaned out of the manager.
    assert(singleton_components_.empty());
}
} // namespace cloud::world::ecs

void cloud::world::ecs::Component::release_singleton_component(
    ComponentManagerData &data)
{
    for (auto ptr : data.singleton_components_)
    {
        if (ptr.second)
        {
            delete ptr.second;
            ptr.second = nullptr;
        }
    }
    data.singleton_components_.clear();
}
