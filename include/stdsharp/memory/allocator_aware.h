#pragma once

#include "allocator_traits.h"

namespace stdsharp
{
    namespace details
    {
        template<typename Alloc, typename Data>
        struct alloc_aware_data : Data
        {
            template<typename... Args>
                requires ::std::constructible_from<Data, Alloc&, Args...> &&
                ::std::constructible_from<Alloc>
            constexpr alloc_aware_data(
                const ::std::allocator_arg_t,
                Alloc& alloc,
                Args&&... args
            ) noexcept(nothrow_constructible_from<Data, Alloc&, Args...>):
                Data(alloc, ::std::forward<Args>(args)...)
            {
            }

            template<typename... Args>
                requires ::std::constructible_from<Data, Args...>
            constexpr alloc_aware_data(const Alloc&, Args&&... args) //
                noexcept(nothrow_constructible_from<Data, Args...>):
                Data(::std::forward<Args>(args)...)
            {
            }

            template<typename... Args>
                requires ::std::constructible_from<Data, Alloc&, Args...> &&
                (!::std::constructible_from<Data, Args...>)
            constexpr alloc_aware_data(Alloc& alloc, Args&&... args) //
                noexcept(nothrow_constructible_from<Data, Alloc&, Args...>):
                Data(alloc, ::std::forward<Args>(args)...)
            {
            }
        };
    }

    template<allocator_req Alloc, typename Data>
    class basic_allocator_aware
    {
    public:
        using traits = allocator_traits<Alloc>;
        using allocator_type = Alloc;
        using data_t = Data;

    private:
        allocator_type allocator_{};
        details::alloc_aware_data<allocator_type, Data> data_;

        template<bool UseAllocator, typename... Args>
        static constexpr auto ctor_req = ::std::constructible_from<Data, Alloc&, Args...> &&
                (UseAllocator || ::std::constructible_from<Alloc>) ?
            nothrow_constructible_from<Data, Alloc&, Args...> &&
                    (UseAllocator || nothrow_constructible_from<Alloc>) ? //
                expr_req::no_exception :
                expr_req::well_formed :
            expr_req::ill_formed;


    public:
        basic_allocator_aware() = default;

        template<typename... Args>
            requires(ctor_req<false, Args...> > expr_req::ill_formed)
        constexpr basic_allocator_aware(Args&&... args) //
            noexcept(ctor_req<false, Args...> == expr_req::no_exception):
            data_(allocator_, ::std::forward<Args>(args)...)
        {
        }

        template<typename... Args>
            requires(ctor_req<true, Args...> > expr_req::ill_formed)
        constexpr basic_allocator_aware(
            const ::std::allocator_arg_t,
            const Alloc& alloc,
            Args&&... args
        ) noexcept(ctor_req<true, Args...> == expr_req::no_exception):
            allocator_(alloc), data_(allocator_, ::std::forward<Args>(args)...)
        {
        }

        template<typename OtherData>
        constexpr basic_allocator_aware(
            const ::std::allocator_arg_t,
            const Alloc& alloc,
            const basic_allocator_aware<Alloc, OtherData>& other
        ) noexcept(ctor_req<true, decltype(other)> == expr_req::no_exception)
            requires(ctor_req<true, decltype(other)> > expr_req::ill_formed)
            : allocator_(alloc), data_(allocator_, other.data_)
        {
        }

        template<typename OtherData>
        constexpr basic_allocator_aware(const basic_allocator_aware<Alloc, OtherData>& other) //
            noexcept(ctor_req<false, decltype(other)> == expr_req::no_exception)
            requires(ctor_req<false, decltype(other)> > expr_req::ill_formed)
            : data_(allocator_, other.data_)
        {
        }

        template<typename OtherData>
        constexpr basic_allocator_aware(
            const ::std::allocator_arg_t,
            const Alloc& alloc,
            basic_allocator_aware<Alloc, OtherData>&& other
        ) noexcept(ctor_req<true, decltype(other)> == expr_req::no_exception)
            requires(ctor_req<true, decltype(other)> > expr_req::ill_formed)
            : allocator_(alloc), data_(allocator_, ::std::move(other.data_))
        {
        }

        template<typename OtherData>
        constexpr basic_allocator_aware(basic_allocator_aware<Alloc, OtherData>&& other) //
            noexcept(ctor_req<false, decltype(other)> == expr_req::no_exception)
            requires(ctor_req<false, decltype(other)> > expr_req::ill_formed)
            : data_(allocator_, ::std::move(other.data_))
        {
        }

    private:
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

        template<typename OtherAlloc, typename OtherData>
        constexpr basic_allocator_aware&
            assign_impl(OtherAlloc&& other_alloc, OtherData&& other_data) noexcept( //
                noexcept( //
                    traits::assign(
                        allocator_,
                        ::std::forward<OtherData>(other_alloc),
                        bind(assign_op, data_, ::std::forward<OtherData>(other_data))
                    )
                )
            )
            requires requires //
        {
            traits::assign(
                allocator_,
                ::std::forward<OtherData>(other_alloc),
                bind(assign_op, data_, ::std::forward<OtherData>(other_data))
            );
        }
        {
            if(&other_data == this) return *this;
            traits::assign(
                allocator_,
                ::std::forward<OtherData>(other_alloc),
                bind(assign_op, data_, ::std::forward<OtherData>(other_data))
            );
            return *this;
        }

    public:
        template<typename OtherData>
        basic_allocator_aware& operator=(const basic_allocator_aware<Alloc, OtherData>& other) //
            noexcept(noexcept(assign_impl(other.allocator_, other.data_)))
            requires requires { assign_impl(other.allocator_, other.data_); }
        {
            return assign_impl(other.allocator_, other.data_);
        }

        template<typename OtherData>
        basic_allocator_aware& operator=(basic_allocator_aware<Alloc, OtherData>&& other) //
            noexcept(noexcept(assign_impl(::std::move(other.allocator_), ::std::move(other.data_))))
            requires requires //
        {
            assign_impl(::std::move(other.allocator_), ::std::move(other.data_)); //
        }
        {
            return assign_impl(::std::move(other.allocator_), ::std::move(other.data_));
        }

    private:
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
        template<typename OtherData>
        constexpr void swap(basic_allocator_aware<Alloc, OtherData>& other) noexcept(
            noexcept(traits::swap(allocator_, other.allocator_, bind(swap_op, data_, other.data_)))
        )
            requires requires //
        {
            traits::swap(allocator_, other.allocator_, bind(swap_op, data_, other.data_)); //
        }
        {
            traits::swap(allocator_, other.allocator_, bind(swap_op, data_, other.data_));
        }

        basic_allocator_aware(const basic_allocator_aware&)
            requires false;
        basic_allocator_aware(basic_allocator_aware&&) noexcept
            requires false;
        basic_allocator_aware& operator=(const basic_allocator_aware&)
            requires false;
        basic_allocator_aware& operator=(basic_allocator_aware&&) noexcept
            requires false;
        ~basic_allocator_aware() = default;

        [[nodiscard]] constexpr auto& allocator() const noexcept { return allocator_; }

        [[nodiscard]] constexpr auto& allocator() noexcept { return allocator_; }

        [[nodiscard]] constexpr const data_t& data() const noexcept { return data_; }

        [[nodiscard]] constexpr data_t& data() noexcept { return data_; }
    };

}