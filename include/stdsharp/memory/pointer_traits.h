#pragma once

#include <memory>

#include "../functional/invocables.h"

namespace stdsharp
{
    namespace details
    {
        template<typename Traits>
        struct pointer_traits_reference :
            ::std::type_identity<decltype(*::std::declval<typename Traits::pointer>())>
        {
        };

        template<typename Traits>
            requires requires { typename Traits::element_type; }
        struct pointer_traits_reference<Traits> :
            ::std::type_identity<::std::add_lvalue_reference_t<typename Traits::element_type>>
        {
        };
    }

    template<class Ptr>
    struct pointer_traits
    {
    private:
        using base = ::std::pointer_traits<Ptr>;

    public:
        using pointer = typename base::pointer;

        using difference_type = typename base::difference_type;

        template<class U>
        using rebind = typename base::template rebind<U>;

        using reference = typename details::pointer_traits_reference<base>::type;

        using element_type = ::std::remove_reference_t<reference>;

        using raw_pointer = element_type*;

    private:
        struct base_pointer_to
        {
            template<class U>
            constexpr pointer operator()(reference r) const noexcept(noexcept(base::pointer_to(r)))
                requires requires { base::pointer_to(r); }
            {
                return base::pointer_to(r);
            }
        };

        struct ptr_pointer_to
        {
            template<typename T = Ptr> // clang-format off
                requires requires(reference r) { { T::pointer_to(r) } -> ::std::same_as<pointer>; } // clang-format on
            constexpr pointer operator()(reference r) const noexcept(noexcept(T::pointer_to(r)))
            {
                return T::pointer_to(r);
            }
        };

        struct convert_pointer_to
        {
            constexpr pointer operator()(reference r) const
                noexcept(nothrow_convertible_to<raw_pointer, pointer>)
                requires ::std::convertible_to<raw_pointer, pointer>
            {
                return static_cast<pointer>(::std::addressof(r));
            }
        };

        using pointer_to_impl_fn =
            sequenced_invocables<base_pointer_to, ptr_pointer_to, convert_pointer_to>;

        static constexpr pointer_to_impl_fn pointer_to_impl;

        struct base_to_address
        {
            constexpr auto operator()(const pointer& p) const
                noexcept(noexcept(base::to_address(p)))
                requires requires { base::to_address(p); }
            {
                return base::to_address(p);
            }
        };

        struct raw_ptr_to_address
        {
            constexpr auto operator()(const pointer& p) const
                noexcept(nothrow_convertible_to<pointer, raw_pointer>)
                requires ::std::convertible_to<pointer, raw_pointer>
            {
                return static_cast<raw_pointer>(p);
            }
        };

        using to_address_impl_fn = sequenced_invocables<base_to_address, raw_ptr_to_address>;

        static constexpr to_address_impl_fn to_address_impl;

    public:
        static constexpr auto pointer_to(element_type& r) //
            noexcept(nothrow_invocable<pointer_to_impl_fn, element_type&>)
            requires ::std::invocable<pointer_to_impl_fn, element_type&>
        {
            return pointer_to_impl(r);
        }

        static constexpr auto to_address(const pointer& p) //
            noexcept(nothrow_invocable<to_address_impl_fn, const pointer&>)
            requires ::std::invocable<to_address_impl_fn, const pointer&>
        {
            return to_address_impl(p);
        }
    };

    inline constexpr struct to_void_pointer_fn
    {
    private:
        template<typename T, typename Traits = pointer_traits<T>>
        static constexpr auto impl(const T& ptr) noexcept
        {
            return static_cast< //
                ::std::conditional_t<
                    const_lvalue_ref<typename Traits::reference>,
                    const void*,
                    void* // clang-format off
                >
            >(Traits::to_address(ptr)); // clang-format on
        }

    public:
        template<typename T>
            requires requires { typename pointer_traits<T>; }
        constexpr auto operator()(const T& ptr) const noexcept(noexcept(impl(ptr)))
        {
            return impl(ptr);
        }
    } to_void_pointer{};
}