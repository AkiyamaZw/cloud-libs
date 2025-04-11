#include "view.h"

namespace cloud::world::ecs {
cloud::world::ecs::ComponentsView::ComponentsView(
    const MetaTypeList &list, internal::Chunk *_chunk,
    const std::vector<internal::Chunk::ChunkComponentMeta *> &lists)
    : selected_comps{std::move(lists)}, chunk(_chunk) {}

View::View(Registry &registry, MaskType mask)
    : registry_(registry), masks_(mask) {}

} // namespace cloud::world::ecs