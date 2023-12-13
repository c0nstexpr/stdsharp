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

        using raw_pointer = std::conditional_t<
            std::same_as<rebind<const element_type>, pointer>,
            std::add_pointer_t<const element_type>,
            std::add_pointer_t<element_type>>;

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
                -> decltype(std::to_address(p))
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
            requires std::invocable<typename pointer_traits<T>::to_address_fn, const T&>
        [[nodiscard]] constexpr auto operator()(const T& ptr) const noexcept
        {
            return (*this)(pointer_traits<T>::to_address(ptr));
        }

        template<typename T>
        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()( //
            const T* const ptr
        ) const noexcept
        {
            return static_cast<const void*>(ptr);
        }

        template<typename T>
        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()(T* const ptr) const noexcept
        {
            return static_cast<void*>(ptr);
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
            static const T* test(const auto*);
            static T* test(auto*);

            using ptr = decltype(test(typename pointer_traits<Pointer>::raw_pointer{}));
        };

    public:
        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()( //
            const void* const ptr
        ) const noexcept
        {
            return static_cast<const T*>(ptr);
        }

        STDSHARP_INTRINSIC [[nodiscard]] constexpr auto operator()( //
            void* const ptr
        ) const noexcept
        {
            return static_cast<T*>(ptr);
        }

        template<typename Pointer>
            requires requires {
                requires std::invocable<to_void_pointer_fn, const Pointer&>;
                requires !nothrow_explicitly_convertible<Pointer, typename traits<Pointer>::ptr>;
            }
        [[nodiscard]] constexpr auto operator()(const Pointer& p) const noexcept
        {
            return (*this)(to_void_pointer(p));
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

    template<typename T>
    struct launder_cast_fn
    {
        template<typename Pointer>
            requires std::invocable<pointer_cast_fn<T>, const Pointer&>
        [[nodiscard]] constexpr auto operator()(const Pointer& p) const noexcept
        {
            return std::launder(pointer_cast<T>(p));
        }
    };

    template<typename T>
    inline constexpr launder_cast_fn<T> launder_cast{};
}

#include "../compilation_config_out.h"