#include "registry.h"
#include <iostream>
#include "entity_manager.h"
using namespace cloud;

namespace cloud::world::ecs
{
struct RegistryData
{
    EntityManagementData entity_mgr_data;
};

} // namespace cloud::world::ecs

int main()
{
    using namespace cloud::world::ecs;

    RegistryData registry_data;
    EntityID id = EntityManager::get_or_create(registry_data.entity_mgr_data);
    std::cout << id.index << std::endl;
    return 0;
}