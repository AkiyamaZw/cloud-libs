#include "selector.h"
#include <cassert>
#ifdef DEBUG
#include <iostream>
#endif
namespace cloud::world::ecs
{

Selector::Selector(RegistryData *registry)
    : registry_ptr(registry)
{
}

Selector Selector::from(RegistryData *registry) { return Selector(registry); }

void Selector::show_chunk_alive_info(Chunk *chunk)
{
#ifdef DEBUG
    std::cout << chunk->entity_live_flags->debug_format_to_string()
              << std::endl;
#endif
}

View Selector::create_view(RegistryData &registry,
                           const MetaTypeList &metas,
                           MaskType mask)
{
    View view{registry, mask};
    Registry::for_each_matching_archetype(
        registry, view.masks_, [&](ArchetypeData *arch) {
            std::vector<ComponentsView> res;
            auto &chunks = Archetype::get_chunks(*arch);
            for (auto chunk : chunks)
            {
                res.emplace_back(metas, chunk, chunk->locate_components(metas));
            }
            view.cached_view_[arch] = std::move(res);
        });
    return view;
}
} // namespace cloud::world::ecs