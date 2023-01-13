#pragma once

#include <memory>

#include "../functional/invocables.h"
#include "../utility/auto_cast.h"

namespace stdsharp
{
    namespace details
    {
        template<typename Traits>
        struct pointer_traits_reference :
            ::std::type_identity<decltype(*::std::declval<typename Traits::pointer>())>
        {
            using element_type = ::std::remove_reference_t<typename pointer_traits_reference::type>;
        };

        template<typename Traits>
            requires ::std::same_as<typename Traits::pointer, void*>
        struct pointer_traits_reference<Traits> : ::std::type_identity<void>
        {
            using element_type = void;
        };

        template<typename Traits>
            requires ::std::same_as<typename Traits::pointer, const void*>
        struct pointer_traits_reference<Traits> : ::std::type_identity<const void>
        {
            using element_type = const void;
        };

        template<typename Traits>
            requires requires { typename Traits::element_type; }
        struct pointer_traits_reference<Traits> :
            ::std::type_identity<::std::add_lvalue_reference_t<typename Traits::element_type>>
        {
            using element_type = ::std::remove_reference_t<typename pointer_traits_reference::type>;
        };
    }

    template<typename Ptr>
    struct pointer_traits : private ::std::pointer_traits<Ptr>
    {
    private:
        using base = ::std::pointer_traits<Ptr>;

    public:
        using typename base::pointer;

        using typename base::difference_type;

        template<typename U>
        using rebind = typename base::template rebind<U>;

        using reference = typename details::pointer_traits_reference<base>::type;

        using element_type = typename details::pointer_traits_reference<base>::element_type;

        using raw_pointer = element_type*;

    private:
        struct base_pointer_to
        {
            template<::std::same_as<reference> T>
                requires requires(T t) //
            {
                requires requires { Ptr::pointer_to(t); } || requires { ::std::addressof(t); }; //
            }
            constexpr pointer operator()(T t) const noexcept
            {
                return base::pointer_to(t);
            }
        };

        struct convert_pointer_to
        {
            constexpr pointer operator()(reference r) const noexcept
                requires nothrow_explicitly_convertible<raw_pointer, pointer>
            {
                return auto_cast(::std::addressof(r));
            }
        };

        using pointer_to_impl_fn = sequenced_invocables<base_pointer_to, convert_pointer_to>;

        static constexpr pointer_to_impl_fn pointer_to_impl;

        struct base_to_address
        {
            constexpr auto operator()(const pointer& p) const noexcept
                requires requires { ::std::to_address(p); }
            {
                return ::std::to_address(p);
            }
        };

        struct raw_ptr_to_address
        {
            constexpr raw_pointer operator()(const pointer& p) const noexcept
                requires nothrow_explicitly_convertible<pointer, raw_pointer>
            {
                return auto_cast(p);
            }
        };

        using to_address_impl_fn = sequenced_invocables<base_to_address, raw_ptr_to_address>;

        static constexpr to_address_impl_fn to_address_impl;

    public:
        static constexpr auto pointer_to(reference r) noexcept
            requires not_same_as<::std::remove_const_t<reference>, void> &&
            ::std::invocable<pointer_to_impl_fn, reference>
        {
            return pointer_to_impl(r);
        }

        static constexpr auto to_address(const pointer& p) //
            noexcept(nothrow_invocable<to_address_impl_fn, const pointer&>)
            requires ::std::invocable<to_address_impl_fn, const pointer&>
        {
            return to_address_impl(p);
        }

        static constexpr pointer to_pointer(const raw_pointer p) // NOLINT(*-misplaced-const)
            noexcept(nothrow_explicitly_convertible<raw_pointer, pointer>)
            requires explicitly_convertible<raw_pointer, pointer>
        {
            return auto_cast(p);
        }

        template<::std::same_as<raw_pointer> T = raw_pointer>
            requires requires(const T p) { pointer_to(*p); }
        static constexpr pointer to_pointer(const T p) // NOLINT(*-misplaced-const)
            noexcept(noexcept(pointer_to(*p)))
        {
            return pointer_to(*p);
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