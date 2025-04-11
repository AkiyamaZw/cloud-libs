#pragma once
#include <type_traits>
#include <tuple>

namespace cloud::world::ecs::internal
{

template <typename T>
struct FunctionTraits;

template <typename T>
using FunctionTraits_args_list = typename FunctionTraits<T>::args_list;

template <typename T>
using FunctionTraits_ret = typename FunctionTraits<T>::ret;

template <typename T>
using FunctionTraits_signature = typename FunctionTraits<T>::signature;

template <typename T>
static constexpr bool is_const = FunctionTraits<T>::is_const;

template <typename T>
static constexpr bool is_valatile = FunctionTraits<T>::is_valatile;

template <typename T>
static constexpr bool is_noexcept = FunctionTraits<T>::is_noexcept;

} // namespace cloud::world::ecs::internal

#include "meta_function_detail.inl"
