#include "selector.h"
#include <cassert>
#ifdef DEBUG
#include <iostream>
#endif
namespace cloud::world::ecs
{

Selector::Selector(Registry *registry)
    : registry_ptr(registry)
{
}

Selector Selector::from(Registry *registry) { return Selector(registry); }

void Selector::show_chunk_alive_info(internal::Chunk *chunk)
{
#ifdef DEBUG
    std::cout << chunk->entity_live_flags->debug_format_to_string()
              << std::endl;
#endif
}

View Selector::create_view(Registry &registry,
                           const MetaTypeList &metas,
                           MaskType mask)
{
    View view{registry, mask};
    registry.for_each_matching_archetype(
        view.masks_, [&](internal::ArchetypeData *arch) {
            std::vector<ComponentsView> res;
            auto &chunks = internal::Archetype::get_chunks(*arch);
            for (auto chunk : chunks)
            {
                res.emplace_back(metas, chunk, chunk->locate_components(metas));
            }
            view.cached_view_[arch] = std::move(res);
        });
    return view;
}
} // namespace cloud::world::ecs