#pragma once

#include "../utility/value_wrapper.h"

namespace stdsharp
{
    template<typename Func>
    struct invocable_t : value_wrapper<Func>
    {
        using base = value_wrapper<Func>;

        using base::value;
        using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                      \
    template<typename... Args>                                              \
        requires ::std::invocable<const_ Func ref, Args...>                 \
    constexpr decltype(auto) operator()(Args&&... args)                     \
        const_ ref noexcept(nothrow_invocable<const_ Func ref, Args...>)    \
    {                                                                       \
        return ::std::invoke(base::value(), ::std::forward<Args>(args)...); \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename Func>
    struct nodiscard_invocable : invocable_t<Func>
    {
        using base = invocable_t<Func>;

        using base::base;
        using base::value;

#define STDSHARP_OPERATOR(const_, ref)                                             \
    template<typename... Args>                                                     \
        requires ::std::invocable<const_ base ref, Args...>                        \
    [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args)              \
        const_ ref noexcept(nothrow_invocable<const_ Func ref, Args...>)           \
    {                                                                              \
        return static_cast<const_ base ref>(*this)(::std::forward<Args>(args)...); \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename Func>
    nodiscard_invocable(Func&& func) -> nodiscard_invocable<::std::decay_t<Func>>;

    template<typename Func>
    struct ref_invocable_t : ::std::reference_wrapper<Func>
    {
        using ::std::reference_wrapper<Func>::reference_wrapper;

#define STDSHARP_OPERATOR(const_, ref)                                   \
    template<typename... Args>                                           \
        requires ::std::invocable<const_ Func ref, Args...>              \
    constexpr decltype(auto) operator()(Args&&... args)                  \
        const_ ref noexcept(nothrow_invocable<const_ Func ref, Args...>) \
    {                                                                    \
        return ::std::invoke(                                            \
            static_cast<const_ Func ref>(this->get()),                   \
            ::std::forward<Args>(args)...                                \
        );                                                               \
    }

        // NOLINTBEGIN(*-exception-escape)
        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)
        // NOLINTEND(*-exception-escape)

#undef STDSHARP_OPERATOR
    };
}