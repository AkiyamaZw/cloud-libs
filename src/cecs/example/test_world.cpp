
#include <iostream>
#include "define.h"
#include "registry.h"
#include "temp/selector.h"
#include "test_with_go.h"
#include "test_meta_function.h"

namespace cloud::world::ecs {

struct Render {};

struct Movement {
  float speed;
  float accelrator;
};

struct Position {
  float x;
  float y;
  float z;
};

class TestWorld {
public:
  void test_entity_info() {
    // ecs::EntityInfoPool einfo_pool;
    // ecs::EntityInfo *einfo = einfo_pool.get_or_create();
    // std::cout << einfo_pool.get_statistic().to_string() << std::endl;
    // einfo_pool.destroy(einfo);
    // std::cout << einfo_pool.get_statistic().to_string() << std::endl;
  }

  void test_Block() {
    std::cout << "test_Block start" << std::endl;
    internal::Block *block = new internal::Block();
    internal::MemoryAcessor<Position> acessor(block, 2);
    acessor[0] = Position(1.0f, 2.0f, 1.0f);
    acessor[1] = Position(3.0f, 1.0f, 3.0f);
    auto print = [](Position *position) {
      std::cout << position->x << " " << position->y << " " << position->z
                << std::endl;
    };
    print(acessor.begin());
    print(acessor.end() - 1);
    Position *p_ptr;
    p_ptr = acessor.begin();
    assert(p_ptr->x == 1.0f && p_ptr->y == 2.0f && p_ptr->z == 1.0f);
    p_ptr = acessor.end() - 1;
    assert(p_ptr->x == 3.0f && p_ptr->y == 1.0f && p_ptr->z == 3.0f);

    delete block;
    std::cout << "test_Block end" << std::endl;
  }

  void test_Chunk() {
    // std::cout << "test_Chunk start" << std::endl;
    // internal::MetaRecord record;
    // auto metas = record.get_metatypes<Position, Movement>();
    // internal::Chunk *chunk = internal::Chunk::create_chunk(metas);
    // chunk->insert(true);
    // chunk->erase(chunk->insert(false));
    // auto accesor_movement = chunk->get_chunk_component<Movement>();
    // auto acessor_position = chunk->get_chunk_component<Position>();

    // for (auto i = acessor_position.begin(); i != acessor_position.end();
    //      ++i)
    // {
    //     std::cout << i->x << " " << i->y << " " << i->z << std::endl;
    // }

    // for (auto i = accesor_movement.begin(); i != accesor_movement.end();
    //      ++i)
    // {
    //     std::cout << i->accelrator << " " << i->speed << std::endl;
    // }

    // internal::Chunk::destroy_chunk(chunk);
    // std::cout << "test_Chunk end" << std::endl;
  }

  void test_Archetype() {
    std::cout << "test_ChunkPool start" << std::endl;
    using namespace cloud::world::ecs;
    Registry registry;
    auto arch = registry.get_or_create_archetype<Position, Movement>();
    auto arch2 = registry.get_or_create_archetype<Movement, Position>();
    assert(arch == arch2);
    registry.destroy_archetype(arch);
    assert(registry.get_archetype_count() == 0);
    std::cout << "test_ChunkPool end" << std::endl;
  }

  void test_Registry() {
    std::cout << "test_Registry end" << std::endl;
    using namespace cloud::world::ecs;
    Registry registry;
    EntityID entity1 = registry.create_entity<Position, Movement>();
    EntityID entity2 = registry.create_entity<Movement, Position>();
    std::vector<EntityID> entities =
        registry.create_entities<Movement, Position>(100);
    auto move = registry.get_component<Movement>(entity1);
    std::cout << move.speed << " " << move.accelrator << std::endl;
    std::cout << "has render? =>" << registry.has_component<Render>(entity1)
              << "has position? =>" << registry.has_component<Position>(entity1)
              << std::endl;

    registry.destroy_entity(entity1);
    {
      auto &select = Selector::from(&registry).with<Position>();
      int count = 0;
      select.for_each([&](Position &position) { count++; });
      std::cout << count << std::endl;
      assert(count == 101);
    }

    registry.destroy_entity(entity2);
    registry.destroy_entities(entities);

    std::cout << "test_Registry end" << std::endl;
  }
};

} // namespace cloud::world::ecs
int main() {
  using namespace cloud::world::ecs;

  TestWorld test;
  // test.test_entity_info();
  // test.test_Block();
  // test.test_Chunk();
  // test.test_Archetype();
  // test.test_Registry();
  vsgo::test_main();
  // test::test_meta_function();
  return 0;
}