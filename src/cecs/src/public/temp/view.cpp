#include "view.h"

namespace cloud::world::ecs
{
cloud::world::ecs::ComponentsView::ComponentsView(
    const MetaTypeList &list,
    Chunk *_chunk,
    const std::vector<Chunk::ChunkComponentMeta *> &lists)
    : selected_comps{std::move(lists)}
    , chunk(_chunk)
{
}

View::View(RegistryData &registry, MaskType mask)
    : registry_(registry)
    , masks_(mask)
{
}

} // namespace cloud::world::ecs