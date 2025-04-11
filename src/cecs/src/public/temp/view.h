#pragma once
#include "chunk.h"
#include "meta_funtion.h"
#include <unordered_map>
#include <iostream>
namespace cloud::world::ecs
{

namespace internal
{
class Archetype;
}

class Registry;

// this is a class is for queried result for a chunk
// the order is define by the MetaTypeList
struct ComponentsView
{
    ComponentsView(
        const MetaTypeList &list,
        internal::Chunk *_chunk,
        const std::vector<internal::Chunk::ChunkComponentMeta *> &lists);

    internal::Chunk *chunk{nullptr};
    // stored in the order of MetaTypeList
    std::vector<internal::Chunk::ChunkComponentMeta *> selected_comps;
};

// template <typename T>
// struct UnPackArgs;

// template <typename... Args>
// struct UnPackArgs<TypeList<Args...>>
// {
//     template <typename Func>
//     static auto invoke(Func &&func, ComponnetPtr &cmp_ptr)
//     {
//         std::apply(func, reinterpret_cast<Args>(cmp.ptr)...);
//     }
// };

/* this class provide multiple ptr to the components that queried by the meta
 * types. It produced by selector, and used by systems iteration update */
class View
{
    friend struct Selector;

  public:
    ~View() = default;

    template <typename Func>
    void each(Func &&func)
    {
        using func_args =
            internal::FunctionTraits_args_list<std::decay_t<Func>>;

        for (auto [arch, views] : cached_view_)
        {
            for (auto view : views)
            {
                each_comps_view(std::forward<Func>(func), func_args{}, view);
            }
        }
    }

    template <typename Func, typename... Args>
    void each_comps_view(Func &&func,
                         internal::TypeList<Args...> &&list,
                         ComponentsView &view)
    {
        auto chunk = view.chunk;
        for (size_t i = 0; i < view.chunk->get_allocated_size(); ++i)
        {
            if (chunk->get_alive(i))
            {
                // auto args = std::tuple{
                //     *(reinterpret_cast<Args *>(view.selected_comps[])[i])...,
                // };
                // std::apply(func, args);
            }
        }
    }

  protected:
    View(Registry &registry, MaskType mask);

  private:
    Registry &registry_;
    MaskType masks_{0};
    std::unordered_map<internal::Archetype *, std::vector<ComponentsView>>
        cached_view_;
};
} // namespace cloud::world::ecs