#pragma once

#include <memory>

#include "../type_traits/object.h"
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
        using element_param_type = std::conditional_t<
            std::same_as<const element_type, const void>,
            private_object<pointer_traits>,
            element_type>;

        struct base_pointer_to
        {
            [[nodiscard]] constexpr pointer operator()(element_param_type& t) const
                noexcept(noexcept(Ptr::pointer_to(t)))
                requires requires { Ptr::pointer_to(t); }
            {
                return base::pointer_to(t);
            }
        };

        struct convert_pointer_to
        {
            [[nodiscard]] constexpr pointer operator()(element_param_type& r) const
                noexcept(nothrow_explicitly_convertible<raw_pointer, pointer>)
                requires explicitly_convertible<raw_pointer, pointer>
            {
                return auto_cast(std::addressof(r));
            }
        };

        struct std_to_address
        {
            [[nodiscard]] constexpr auto operator()(const pointer& p) const noexcept
                requires requires { std::to_address(p); }
            {
                return std::to_address(p);
            }
        };

        struct convert_to_address
        {
            [[nodiscard]] STDSHARP_INTRINSIC constexpr raw_pointer operator()( //
                const pointer& p
            ) const noexcept
                requires nothrow_explicitly_convertible<pointer, raw_pointer>
            {
                return static_cast<raw_pointer>(p);
            }
        };

    public:
        using pointer_to_fn = sequenced_invocables<base_pointer_to, convert_pointer_to>;

        static constexpr pointer_to_fn pointer_to{};

        using to_address_fn = sequenced_invocables<std_to_address, convert_to_address>;

        static constexpr to_address_fn to_address{};

    private:
        struct dereference_to_pointer
        {
            [[nodiscard]] constexpr pointer operator()(const raw_pointer p) const
                noexcept(noexcept(pointer_to(*p), pointer{}))
                requires requires {
                    requires nullable_pointer<pointer>;
                    pointer_to(*p);
                }
            {
                return p == nullptr ? pointer{} : pointer_to(*p);
            }
        };

        struct convert_to_pointer
        {
            [[nodiscard]] STDSHARP_INTRINSIC constexpr pointer operator()(const raw_pointer p) const
                noexcept(nothrow_explicitly_convertible<raw_pointer, pointer>)
                requires explicitly_convertible<raw_pointer, pointer>
            {
                return static_cast<pointer>(p);
            }
        };

    public:
        using to_pointer_fn = sequenced_invocables<dereference_to_pointer, convert_to_pointer>;

        static constexpr to_pointer_fn to_pointer{};
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
            requires requires {
                requires std::invocable<to_void_pointer_fn, const Pointer&>;
                requires !nothrow_explicitly_convertible<Pointer, typename traits<Pointer>::ptr>;
            }
        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()(const Pointer& p) const noexcept
        {
            return static_cast<typename traits<Pointer>::ptr>(to_void_pointer(p));
        }

        template<typename Pointer>
            requires nothrow_explicitly_convertible<Pointer, typename traits<Pointer>::ptr>
        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()(const Pointer& p) const noexcept
        {
            return static_cast<typename traits<Pointer>::ptr>(p);
        }
    };

    template<typename T>
    inline constexpr pointer_cast_fn<T> pointer_cast{};
}

#include "../compilation_config_out.h"