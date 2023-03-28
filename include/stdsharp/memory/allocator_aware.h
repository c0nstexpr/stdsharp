#pragma once

#include "allocator_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc, typename Data>
    struct basic_allocator_aware
    {
        using traits = allocator_traits<Alloc>;

        Alloc allocator;
        Data data;

    private:
        static constexpr struct get_alloc_fn
        {
            template<typename... Args>
            constexpr Alloc operator()(Data& d, Args&&... args) const
                noexcept(noexcept(d.get_default_allocator(::std::declval<Args>()...)))
                requires requires // clang-format off
            {
                { d.get_default_allocator(::std::declval<Args>()...) } -> // clang-format on
                        ::std::same_as<Alloc>;
            }
            {
                return d.get_default_allocator(::std::forward<Args>(args)...);
            }

            constexpr auto operator()(const auto&...) const
                noexcept(nothrow_constructible_from<Alloc>)
                requires ::std::constructible_from<Alloc>
            {
                return Alloc{};
            }
        } get_alloc{};

        static constexpr struct assign_op_fn
        {
            template<typename OtherData, allocator_assign_operation assign, typename OtherAlloc>
            constexpr void operator()(
                Data& d,
                OtherData&& other,
                const constant<assign>,
                Alloc& left,
                OtherAlloc&& right
            ) const noexcept( //
                noexcept( //
                    d.template assign_allocator<assign>(
                        ::std::forward<OtherData>(other),
                        left,
                        ::std::forward<OtherAlloc>(right)
                    )
                )
            )
                requires requires //
            {
                d.template assign_allocator<assign>(
                    ::std::forward<OtherData>(other),
                    left,
                    ::std::forward<OtherAlloc>(right)
                ); //
            }
            {
                d.template assign_allocator<assign>(
                    ::std::forward<OtherData>(other),
                    left,
                    ::std::forward<OtherAlloc>(right)
                );
            }

            template<typename OtherData, typename OtherAlloc>
            constexpr void
                operator()(Data& d, OtherData&& other, Alloc& left, OtherAlloc&& right) const
                noexcept( //
                    noexcept( //
                        d.assign_allocator(
                            ::std::forward<OtherData>(other),
                            left,
                            ::std::forward<OtherAlloc>(right)
                        )
                    )
                )
                requires requires //
            {
                d.assign_allocator(
                    ::std::forward<OtherData>(other),
                    left,
                    ::std::forward<OtherAlloc>(right)
                );
            }
            {
                d.assign_allocator(
                    ::std::forward<OtherData>(other),
                    left,
                    ::std::forward<OtherAlloc>(right)
                );
            }
        } assign_op{};

        static constexpr struct swap_op_fn
        {
            template<typename OtherData, allocator_swap_operation swap>
            constexpr void operator()(
                Data& d,
                OtherData& other,
                const constant<swap>,
                Alloc& left,
                Alloc& right
            ) const noexcept(noexcept(d.template swap_allocator<swap>(other, left, right)))
                requires requires { d.template swap_allocator<swap>(other, left, right); }
            {
                d.template swap_allocator<swap>(other, left, right);
            }

            template<typename OtherData>
            constexpr void operator()(Data& d, OtherData& other, Alloc& left, Alloc& right) const
                noexcept(noexcept(d.swap_allocator(other, left, right)))
                requires requires { d.swap_allocator(other, left, right); }
            {
                d.swap_allocator(other, left, right);
            }
        } swap_op{};

    public:
        basic_allocator_aware() = default;

        template<typename... Args>
            requires ::std::constructible_from<Data, Alloc&, Args...> &&
                         ::std::invocable<
                             get_alloc_fn,
                             Data&,
                             Args...>
        constexpr basic_allocator_aware(Args&&... args) //
            noexcept(nothrow_constructible_from<Data, Alloc&, Args...>&&
                         nothrow_invocable<get_alloc_fn, Data&, Args...>):
            allocator(get_alloc(::std::forward<Args>(args)...)),
            data(allocator, ::std::forward<Args>(args)...)
        {
        }

        template<typename... Args>
            requires ::std::constructible_from<Data, Alloc&, Args...>
        constexpr basic_allocator_aware(
            const ::std::allocator_arg_t,
            const Alloc& alloc,
            Args&&... args
        ) //
            noexcept(nothrow_constructible_from<Data, Alloc&, Args...>):
            allocator(alloc), data(allocator, ::std::forward<Args>(args)...)
        {
        }

        template<typename OtherData>
            requires ::std::constructible_from<Data, Alloc&, const OtherData&>
        constexpr basic_allocator_aware(
            const ::std::allocator_arg_t,
            const Alloc& alloc,
            const basic_allocator_aware<Alloc, OtherData>& other
        ) //
            noexcept(nothrow_constructible_from<Data, Alloc&, const OtherData&>):
            allocator(alloc), data(allocator, other.data)
        {
        }

        template<typename OtherData>
            requires ::std::constructible_from<Data, Alloc&, const OtherData&>
        constexpr basic_allocator_aware(const basic_allocator_aware<Alloc, OtherData>& other) //
            noexcept(nothrow_constructible_from<Data, Alloc&, const OtherData&>):
            basic_allocator_aware(
                ::std::allocator_arg,
                traits::copy_construct(other.allocator),
                other
            )
        {
        }

        template<typename OtherData>
            requires ::std::constructible_from<Data, Alloc&, OtherData>
        constexpr basic_allocator_aware(
            const ::std::allocator_arg_t,
            const Alloc& alloc,
            basic_allocator_aware<Alloc, OtherData>&& other
        ) //
            noexcept(nothrow_constructible_from<Data, Alloc&, OtherData>):
            allocator(alloc), data(allocator, ::std::move(other.data))
        {
        }

        template<typename OtherData>
            requires ::std::constructible_from<Data, Alloc&, OtherData>
        constexpr basic_allocator_aware(basic_allocator_aware<Alloc, OtherData>&& other) //
            noexcept(nothrow_constructible_from<Data, Alloc&, OtherData>):
            basic_allocator_aware(
                ::std::allocator_arg,
                ::std::move(other.allocator),
                ::std::move(other)
            )
        {
        }

    private:
        template<typename OtherAlloc, typename OtherData>
        constexpr basic_allocator_aware&
            assign_impl(OtherAlloc&& other_alloc, OtherData&& other_data) noexcept( //
                noexcept( //
                    traits::assign(
                        allocator,
                        ::std::forward<OtherData>(other_alloc),
                        bind(assign_op, data, ::std::forward<OtherData>(other_data))
                    )
                )
            )
            requires requires //
        {
            traits::assign(
                allocator,
                ::std::forward<OtherData>(other_alloc),
                bind(assign_op, data, ::std::forward<OtherData>(other_data))
            );
        }
        {
            if(&other_data == this) return *this;
            traits::assign(
                allocator,
                ::std::forward<OtherData>(other_alloc),
                bind(assign_op, data, ::std::forward<OtherData>(other_data))
            );
            return *this;
        }

    public:
        template<typename OtherData>
        basic_allocator_aware& operator=(const basic_allocator_aware<Alloc, OtherData>& other) //
            noexcept(noexcept(assign_impl(other.allocator, other.data)))
            requires requires { assign_impl(other.allocator, other.data); }
        {
            return assign_impl(other.allocator, other.data);
        }

        template<typename OtherData>
        basic_allocator_aware& operator=(basic_allocator_aware<Alloc, OtherData>&& other) //
            noexcept(noexcept(assign_impl(::std::move(other.allocator), ::std::move(other.data))))
            requires requires //
        {
            assign_impl(::std::move(other.allocator), ::std::move(other.data)); //
        }
        {
            return assign_impl(::std::move(other.allocator), ::std::move(other.data));
        }

        template<typename OtherData>
        constexpr void swap(basic_allocator_aware<Alloc, OtherData>& other) noexcept(
            noexcept(traits::swap(allocator, other.allocator, bind(swap_op, data, other.data)))
        )
            requires requires //
        {
            traits::swap(allocator, other.allocator, bind(swap_op, data, other.data)); //
        }
        {
            traits::swap(allocator, other.allocator, bind(swap_op, data, other.data));
        }

        basic_allocator_aware(const basic_allocator_aware&)
            requires false;
        basic_allocator_aware(basic_allocator_aware&&) noexcept
            requires false;
        basic_allocator_aware& operator=(const basic_allocator_aware&) = default;
        basic_allocator_aware& operator=(basic_allocator_aware&&) noexcept = default;
        ~basic_allocator_aware() = default;

        [[nodiscard]] constexpr auto get_allocator() const noexcept { return allocator; }
    };

}