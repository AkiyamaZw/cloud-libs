#pragma once

namespace cloud::world::ecs::internal
{
enum class RefMode
{
    None,
    Left,
    Right,
};

template <typename... T>
struct TypeList
{
};

template <bool IS_CONST, bool IS_VALATILE, bool IS_NOEXCEPT, RefMode REF, typename Signature>
struct FunctionTraitsBasic;

template <bool IS_CONST,
          bool IS_VALATILE,
          bool IS_NOEXCEPT,
          RefMode REF,
          typename Ret,
          typename... Args>
struct FunctionTraitsBasic<IS_CONST, IS_VALATILE, IS_NOEXCEPT, REF, Ret(Args...)>
{
    using args_list = TypeList<Args...>;
    using ret = Ret;
    using signature = Ret(Args...);
    static constexpr bool is_const = IS_CONST;
    static constexpr bool is_valatile = IS_VALATILE;
    static constexpr bool is_noexcept = IS_NOEXCEPT;
    static constexpr RefMode ref = REF;
};

template <bool is_function, typename T>
struct FunctionTraitsDiscriminator;

template <typename T>
struct FunctionTraitsDiscriminator<false, T>
    : FunctionTraits<decltype(&std::decay_t<T>::operator())>
{
    // static_assert(false, typeid(decltype(&std::decay_t<T>::operator())).name());
};

template <typename T>
struct FunctionTraitsDiscriminator<true, T> : FunctionTraits<T>
{
    using Function = T;
};

// ↓ ↓ ↓ function dispatcher ↓ ↓ ↓
//  normal func pointer
template <typename Func>
struct FunctionTraits<Func *> : FunctionTraits<Func>
{
    using Object = void;
    using Function = Func;
};

// member func pointer
template <typename Kls, typename Func>
struct FunctionTraits<Func Kls::*> : FunctionTraits<Func>
{
    using Object = Kls;
    using Function = Func;
};

// func ref
template <typename Func>
struct FunctionTraits<Func &> : FunctionTraits<Func>
{
};

// func right ref
template <typename Func>
struct FunctionTraits<Func &&> : FunctionTraits<Func>
{
};

// no function dispatcher
template <typename T>
struct FunctionTraits : FunctionTraitsDiscriminator<std::is_function_v<T>, T>
{
};
// ↑ ↑ ↑ function dispatcher ↑ ↑ ↑

// ↓ ↓ ↓ matcher function ↓ ↓ ↓
// |0        |0            |0            |
// |is const | is_volatile | is_noexcept |

// ↓ ↓ ↓ no ref ↓ ↓ ↓
// 000
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...)>
    : public FunctionTraitsBasic<false, false, false, RefMode::None, Ret(Args...)>
{
};

// 100
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const>
    : public FunctionTraitsBasic<true, false, false, RefMode::None, Ret(Args...)>
{
};

// 010
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) volatile>
    : public FunctionTraitsBasic<false, true, false, RefMode::None, Ret(Args...)>
{
};

// 001
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) noexcept>
    : public FunctionTraitsBasic<false, false, true, RefMode::None, Ret(Args...)>
{
};
// 110
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const volatile>
    : public FunctionTraitsBasic<true, true, false, RefMode::None, Ret(Args...)>
{
};
// 101
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const noexcept>
    : public FunctionTraitsBasic<true, false, true, RefMode::None, Ret(Args...)>
{
};

// 011
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) volatile noexcept>
    : public FunctionTraitsBasic<false, true, true, RefMode::None, Ret(Args...)>
{
};

// 111
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const volatile noexcept>
    : public FunctionTraitsBasic<true, true, true, RefMode::None, Ret(Args...)>
{
};
// ↑ ↑ ↑ no ref ↑ ↑ ↑

// ↓ ↓ ↓ ref ↓ ↓ ↓

// 000
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) &>
    : public FunctionTraitsBasic<false, false, false, RefMode::Left, Ret(Args...)>
{
};

// 100
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const &>
    : public FunctionTraitsBasic<true, false, false, RefMode::Left, Ret(Args...)>
{
};

// 010
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) volatile &>
    : public FunctionTraitsBasic<false, true, false, RefMode::Left, Ret(Args...)>
{
};

// 001
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) & noexcept>
    : public FunctionTraitsBasic<false, false, true, RefMode::Left, Ret(Args...)>
{
};
// 110
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const volatile &>
    : public FunctionTraitsBasic<true, true, false, RefMode::Left, Ret(Args...)>
{
};
// 101
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const & noexcept>
    : public FunctionTraitsBasic<true, false, true, RefMode::Left, Ret(Args...)>
{
};

// 011
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) volatile & noexcept>
    : public FunctionTraitsBasic<false, true, true, RefMode::Left, Ret(Args...)>
{
};

// 111
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const volatile & noexcept>
    : public FunctionTraitsBasic<true, true, true, RefMode::Left, Ret(Args...)>
{
};
// ↑ ↑ ↑ ref ↑ ↑ ↑

// ↓ ↓ ↓ right ref ↓ ↓ ↓
// 000
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) &&>
    : public FunctionTraitsBasic<false, false, false, RefMode::Right, Ret(Args...)>
{
};

// 100
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const &&>
    : public FunctionTraitsBasic<true, false, false, RefMode::Right, Ret(Args...)>
{
};

// 010
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) volatile &&>
    : public FunctionTraitsBasic<false, true, false, RefMode::Right, Ret(Args...)>
{
};

// 001
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) && noexcept>
    : public FunctionTraitsBasic<false, false, true, RefMode::Right, Ret(Args...)>
{
};
// 110
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const volatile &&>
    : public FunctionTraitsBasic<true, true, false, RefMode::Right, Ret(Args...)>
{
};
// 101
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const && noexcept>
    : public FunctionTraitsBasic<true, false, true, RefMode::Right, Ret(Args...)>
{
};

// 011
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) volatile && noexcept>
    : public FunctionTraitsBasic<false, true, true, RefMode::Right, Ret(Args...)>
{
};

// 111
template <typename Ret, typename... Args>
struct FunctionTraits<Ret(Args...) const volatile && noexcept>
    : public FunctionTraitsBasic<true, true, true, RefMode::Right, Ret(Args...)>
{
};
// ↑ ↑ ↑ right ref ↑ ↑ ↑
// ↑ ↑ ↑ matcher ↑ ↑ ↑
} // namespace cloud::world::ecs::internal
