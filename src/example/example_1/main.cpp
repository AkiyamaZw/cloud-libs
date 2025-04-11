#include "registry.h"

using namespace cloud;

struct AComp
{
    int a;
};

struct BComp
{
    int b;
};

int main()
{
    world::ecs::Registry reg;
    auto ett = reg.create_entities<AComp, BComp>(10);
    return 0;
}