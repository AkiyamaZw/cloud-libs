#pragma once
#include <unordered_map>
#include <functional>
#include "define.h"

namespace cloud::world::ecs {
namespace internal {
class Archetype;
}
class ArchetypeManager final {
public:
  ArchetypeManager() = default;
  ~ArchetypeManager();

  ArchetypeManager(const ArchetypeManager &) = delete;
  ArchetypeManager(ArchetypeManager &&) = delete;
  ArchetypeManager &operator=(const ArchetypeManager &) = delete;
  ArchetypeManager &operator==(ArchetypeManager &&) = delete;

  // called when system shutdown
  void release_archetype();

  // get an archetype
  internal::Archetype *get_archetype(MaskType mask);

  // create and archetype
  internal::Archetype *create_archetype(MaskType mask,
                                        MetaTypeList meta_type_list);

  // each archetype operator
  void
  for_each_matching_archetype(MaskType mask,
                              std::function<void(internal::Archetype *)> cb);

  // delete an archetype
  bool destroy_archetype(internal::Archetype *archetype);

  // get count
  size_t get_archetype_count() const;

private:
  friend struct Selector;
  std::unordered_map<MaskType, internal::Archetype *> archetypes_;
};
} // namespace cloud::world::ecs