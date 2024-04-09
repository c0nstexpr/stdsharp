#pragma once

#include "invoke.h"
#include "project_invoke.h"
#include "sequenced_invocables.h"

namespace stdsharp::details
{
    template<typename Func, typename... BindArgs>
    struct perfect_bind_front
    {
    private:
        using indexed_values = indexed_values<std::conditional_t<
            lvalue_ref<BindArgs>,
            std::reference_wrapper<std::remove_reference_t<BindArgs>>,
            std::remove_cvref_t<BindArgs>>...>;

        template<std::size_t I, typename Self>
        constexpr decltype(auto) unref(this Self&& self) noexcept
        {
            auto&& bind_args = forward_cast<Self, perfect_bind_front>(self).bind_args;
            if constexpr(lvalue_ref<type_at<I, BindArgs...>>) return cpp_forward(bind_args).get();
            else return cpp_forward(bind_args);
        }

        template<std::size_t I, typename Self>
        using unref_t = decltype(std::declval<Self>().template unref<I>());

    public:
        Func func;
        indexed_values bind_args;

    private:
        template<typename Self, typename... Args, std::size_t... I>
            requires std::invocable<forward_cast_t<Self, Func>, unref_t<I, Self>..., Args...>
        constexpr decltype(auto) impl(
            this Self&& self,
            Args&&... args,
            const std::index_sequence<I...> /*unused*/
        ) noexcept(nothrow_invocable<forward_cast_t<Self, Func>, unref_t<I, Self>..., Args...>)
        {
            auto&& this_ = forward_cast<Self, perfect_bind_front>(self);
            return invoke(
                cpp_forward(this_).func,
                cpp_forward(this_).template unref<I>()...,
                cpp_forward(args)...
            );
        }

    public:
        template<typename Self, typename... Args>
        constexpr decltype(auto) operator()(Self&& self, Args&&... args) noexcept( //
            noexcept( //
                forward_cast<Self, perfect_bind_front>(self)
                    .impl(cpp_forward(args)..., std::index_sequence_for<BindArgs...>{})
            )
        )
            requires requires {
                forward_cast<Self, perfect_bind_front>(self).impl(
                    cpp_forward(args)...,
                    std::index_sequence_for<BindArgs...>{}
                );
            }
        {
            return forward_cast<Self, perfect_bind_front>(self).impl(
                cpp_forward(args)...,
                std::index_sequence_for<BindArgs...>{}
            );
        }
    };
}

namespace stdsharp
{
    template<typename... Args>
    struct deduction<details::perfect_bind_front, Args...>
    {
        using type = details::perfect_bind_front<Args...>;
    };

    using perfect_bind_front_fn = make_template_type_fn<details::perfect_bind_front>;

    inline constexpr perfect_bind_front_fn perfect_bind_front{};
}