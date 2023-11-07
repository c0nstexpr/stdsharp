#pragma once

#include <memory>

#include "../functional/invocables.h"
#include "../utility/auto_cast.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename Ptr>
    struct pointer_traits : private std::pointer_traits<Ptr>
    {
    private:
        using base = std::pointer_traits<Ptr>;

    public:
        using typename base::pointer;

        using typename base::difference_type;

        using typename base::element_type;

        template<typename U>
        using rebind = base::template rebind<U>;

        using reference = std::add_lvalue_reference_t<element_type>;

        using raw_pointer = std::add_pointer_t<element_type>;

    private:
        struct base_pointer_to
        {
            constexpr pointer operator()(std::same_as<reference> auto&& t) const
                noexcept(noexcept(Ptr::pointer_to(t)))
                requires requires { Ptr::pointer_to(t); }
            {
                return base::pointer_to(t);
            }
        };

        struct convert_pointer_to
        {
            constexpr pointer operator()(std::same_as<reference> auto&& r) const
                noexcept(nothrow_explicitly_convertible<raw_pointer, pointer>)
                requires explicitly_convertible<raw_pointer, pointer>
            {
                return auto_cast(std::addressof(r));
            }
        };

        using pointer_to_fn = sequenced_invocables<base_pointer_to, convert_pointer_to>;

        static constexpr pointer_to_fn pointer_to_impl;

        struct std_to_address
        {
            constexpr auto operator()(const pointer& p) const noexcept
                requires requires { std::to_address(p); }
            {
                return std::to_address(p);
            }
        };

        struct convert_to_address
        {
            STDSHARP_INTRINSIC constexpr raw_pointer operator()(const pointer& p) const noexcept
                requires nothrow_explicitly_convertible<pointer, raw_pointer>
            {
                return static_cast<raw_pointer>(p);
            }
        };

        using to_address_fn = sequenced_invocables<std_to_address, convert_to_address>;

        static constexpr to_address_fn to_address_impl;

    public:
        [[nodiscard]] static constexpr auto pointer_to(std::same_as<element_type> auto& r) //
            noexcept(nothrow_invocable<pointer_to_fn, reference>)
            requires std::invocable<pointer_to_fn, reference>
        {
            return pointer_to_impl(r);
        }

        [[nodiscard]] static constexpr auto to_address(const pointer& p) noexcept
            requires std::invocable<to_address_fn, const pointer&>
        {
            return to_address_impl(p);
        }

    private:
        struct identity_to_pointer
        {
            constexpr pointer operator()(const pointer& p) const noexcept { return p; }
        };

        struct dereference_to_pointer
        {
            constexpr pointer operator()(const std::same_as<raw_pointer> auto p) const
                noexcept(noexcept(pointer_to(*p)))
                requires requires { pointer_to(*p); }
            {
                return pointer_to(*p);
            }
        };

        struct convert_to_pointer
        {
            STDSHARP_INTRINSIC constexpr pointer operator()( //
                const raw_pointer p // NOLINT(*-misplaced-const)
            ) const noexcept(nothrow_explicitly_convertible<raw_pointer, pointer>)
                requires explicitly_convertible<raw_pointer, pointer>
            {
                return static_cast<pointer>(p);
            }
        };

        using to_pointer_fn =
            sequenced_invocables<identity_to_pointer, dereference_to_pointer, convert_to_pointer>;

        static constexpr to_pointer_fn to_pointer_impl;

    public:
        [[nodiscard]] static constexpr auto to_pointer( //
            const raw_pointer p // NOLINT(*-misplaced-const)
        ) noexcept(nothrow_invocable<to_pointer_fn, const raw_pointer>)
            requires std::invocable<to_pointer_fn, const raw_pointer>
        {
            return to_pointer_impl(p);
        }
    };

    inline constexpr struct to_void_pointer_fn
    {
        template<typename T>
            requires requires { typename pointer_traits<T>; }
        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()(const T& ptr) const noexcept
        {
            using traits = pointer_traits<T>;
            using ret =
                std::conditional_t<const_<typename traits::element_type>, const void*, void*>;

            return static_cast<ret>(traits::to_address(ptr));
        }
    } to_void_pointer{};

    template<typename T>
    struct pointer_cast_fn
    {
    private:
        template<typename Pointer>
            requires requires { pointer_traits<Pointer>{}; }
        struct traits
        {
            using ptr = std::
                conditional_t<const_<typename pointer_traits<Pointer>::element_type>, const T*, T*>;
        };

    public:
        template<typename Pointer>
            requires requires //
        {
            traits<Pointer>{};
            requires std::invocable<to_void_pointer_fn, const Pointer&>;
            requires !explicitly_convertible<Pointer, typename traits<Pointer>::ptr>;
        }
        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()(const Pointer& p) const noexcept
        {
            return static_cast<typename traits<Pointer>::ptr>(to_void_pointer(p));
        }

        template<typename Pointer>
            requires requires //
        {
            traits<Pointer>{};
            requires explicitly_convertible<Pointer, typename traits<Pointer>::ptr>;
        }
        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()(const Pointer& p) const noexcept
        {
            return static_cast<typename traits<Pointer>::ptr>(p);
        }
    };

    template<typename T>
    inline constexpr pointer_cast_fn<T> pointer_cast{};
}

#include "../compilation_config_out.h"