#pragma once

#include "invoke.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    inline constexpr struct bind_lvalue_fn
    {
    private:
        template<typename T>
        struct lvalue_binder : std::reference_wrapper<T>
        {
            using std::reference_wrapper<T>::reference_wrapper;
        };

        struct binder_projector
        {
            template<typename T>
                requires std::same_as<T, lvalue_binder<typename T::type>>
            constexpr decltype(auto) operator()(const T& value) const noexcept
            {
                return value.get();
            }
        };

        using projector = sequenced_invocables<binder_projector, std::identity>;

        template<typename T>
        static constexpr decltype(auto) wrap(T&& value) noexcept
        {
            if constexpr(lvalue_ref<T&&>) return lvalue_binder<std::remove_reference_t<T>>{value};
            else return cpp_forward(value);
        }

    public:
        template<typename... Args>
        constexpr auto operator()(auto&& func, Args&&... args) const noexcept( //
            noexcept( //
                std::bind_front(
                    projected_invoke,
                    cpp_forward(func),
                    projector{},
                    wrap(cpp_forward(args))...
                )
            )
        )
            requires requires {
                std::bind_front(
                    projected_invoke,
                    cpp_forward(func),
                    projector{},
                    wrap(cpp_forward(args))...
                );
            }
        {
            return std::bind_front(
                projected_invoke,
                cpp_forward(func),
                projector{},
                wrap(cpp_forward(args))...
            );
        }
    } bind_lvalue{};
}

#include "../compilation_config_out.h"