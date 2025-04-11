#pragma once
#include <vector>
#include "meta_type.h"
#include "registry.h"
#include <cassert>
#include <functional>
#include <tuple>
#include "view.h"

namespace cloud::world::ecs
{
template <typename... T>
struct type_list
{
};

template <typename Kls, typename Ret, typename... Args>
type_list<Args...> args(Ret (Kls::*)(Args...) const);

struct Selector
{
    Selector() = default;
    Selector(Registry *registry);
    Registry *registry_ptr{nullptr};
    MetaTypeList required_comps{};
    MaskType required_archetype{0};
    MetaTypeList exclude_comps{};

    // just a factory function
    static Selector from(Registry *registry);

    template <typename... C>
    Selector &with()
    {
        assert(registry_ptr != nullptr);
        auto metas = registry_ptr->component_manager_->get_metatypes<C...>();
        required_comps.insert(required_comps.end(), metas.begin(), metas.end());
        (...,
         (required_archetype.set(
             registry_ptr->component_manager_->get_or_register<C>())));
        return *this;
    }

    template <typename... Args, typename Func>
    void unpack_chunk(type_list<Args...> type,
                      internal::Chunk *chunk,
                      Func &&func)
    {
        // #ifdef DEBUG
        //         show_chunk_alive_info(chunk);
        // #endif
        auto tup = std::make_tuple(chunk->get_chunk_component<Args>()...);
        for (size_t i = 0; i < chunk->get_allocated_size(); ++i)
        {
            if (chunk->get_alive(i))
            {
                func(std::get<decltype(chunk->get_chunk_component<Args>())>(
                    tup)[i]...);
            }
        }
    }

    template <typename Func>
    Selector &for_each(Func &&func)
    {
        using Params = decltype(args(&Func::operator()));
        for (auto [mask, archetype] :
             registry_ptr->archetype_manager_->archetypes_)
        {
            if ((mask & required_archetype) != required_archetype)
                continue;
            for (auto chunk : archetype->get_chunks())
            {
                unpack_chunk(Params{}, chunk, std::forward<Func>(func));
            }
        }
        return *this;
    }

    void show_chunk_alive_info(internal::Chunk *chunk);

    template <typename... C>
    static View query(Registry &registry)
    {

        auto metas = registry.component_manager_->get_metatypes<C...>();
        MaskType mask;
        (..., (mask.set(registry.component_manager_->get_or_register<C>())));

        return Selector::create_view(registry, metas, mask);
    }

    static View create_view(Registry &registry,
                            const MetaTypeList &metas,
                            MaskType mask);
};

} // namespace cloud::world::ecs
