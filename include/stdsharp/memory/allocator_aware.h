#pragma once

#include <span>

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../cassert/cassert.h"
#include "../cstdint/cstdint.h"

namespace stdsharp
{
    enum class allocator_assign_operation
    {
        before_assign,
        after_assign
    };

    enum class allocator_swap_operation
    {
        before_swap,
        after_swap
    };

    template<typename Alloc>
    struct allocator_aware_traits : allocator_traits<Alloc>
    {
        using allocator_traits = allocator_traits<Alloc>;

        using typename allocator_traits::value_type;
        using typename allocator_traits::pointer;
        using typename allocator_traits::const_void_pointer;
        using typename allocator_traits::size_type;
        using typename allocator_traits::allocator_type;
        using typename allocator_traits::propagate_on_container_move_assignment;
        using typename allocator_traits::propagate_on_container_copy_assignment;
        using typename allocator_traits::propagate_on_container_swap;

        using enum allocator_assign_operation;
        using enum allocator_swap_operation;

        struct allocation
        {
        private:
            template<
                typename T,
                auto Impl,
                typename Fn = decltype(Impl),
                typename ValueType = ::std::conditional_t<::std::is_void_v<T>, value_type, T>>
            struct do_as_fn
            {
                static constexpr bool is_invocable = ::std::invocable<
                    Fn,
                    allocator_type&,
                    const allocation&,
                    const ::std::span<ValueType>&>;

                static constexpr bool is_noexcept = nothrow_invocable<
                    Fn,
                    allocator_type&,
                    const allocation&,
                    const ::std::span<ValueType>&>;

                constexpr void operator()(
                    allocator_type& alloc,
                    const allocation& dest,
                    const allocation& src,
                    const size_type count
                ) const noexcept(is_noexcept)
                    requires is_invocable
                {
                    Impl(alloc, dest, src.template as_span<ValueType>(count));
                }

                constexpr void
                    operator()(allocator_type& alloc, const allocation& dest, const allocation& src)
                        const noexcept(is_noexcept)
                    requires is_invocable
                {
                    Impl(alloc, dest, src.template as_span<ValueType>());
                }
            };

            template<typename T = void>
            struct copy_as_impl_fn
            {
                using value_t = ::std::conditional_t<::std::is_void_v<T>, value_type, T>;

                constexpr void operator()(
                    allocator_type& alloc,
                    const allocation& dest,
                    const ::std::span<value_t>& src_span
                ) const
                {
                    const auto& dest_span = dest.template as_span<value_t>();
                    auto d_first = dest_span.begin();

                    for(const auto& v : src_span) allocator_traits::construct(alloc, d_first++, v);
                }
            };

            template<typename T = void>
            struct assign_as_impl_fn
            {
                using value_t = ::std::conditional_t<::std::is_void_v<T>, value_type, T>;

                constexpr void
                    operator()(const allocation& dest, const ::std::span<value_t>& src_span) const
                    noexcept(nothrow_copy_constructible<value_t>)
                {
                    const auto& dest_span = dest.template as_span<value_t>();
                    auto d_first = dest_span.begin();

                    for(const auto& v : src_span) *(d_first++) = v;
                }
            };

        public:
            template<typename T = void>
            using copy_as_fn = do_as_fn<T, copy_as_impl_fn<T>{}>;

            template<typename T = void>
            static constexpr copy_as_fn copy_as{};

            template<typename T = void>
            using assign_as_fn = do_as_fn<T, assign_as_impl_fn<T>{}>;

            template<typename T = void>
            static constexpr assign_as_fn assign_as{};

        private:
            pointer ptr_{};
            size_type size_{};

        public:
            [[nodiscard]] constexpr auto size() const noexcept { return size_; }

            [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

            [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

            [[nodiscard]] constexpr auto& operator[](const size_type index) const noexcept
            {
                return ptr_[index];
            }

            constexpr void allocate(allocator_type& alloc, const size_type size) const noexcept
            {
                if(size_ >= size) return;

                if(ptr_ != nullptr) deallocate(alloc);

                *this = allocator_traits::get_allocation(size);
            }

            constexpr void deallocate(allocator_type& alloc) const noexcept
            {
                allocator_traits::deallocate(alloc, ptr_, size_);
            }

            constexpr void destroy(allocator_type& alloc) const noexcept
            {
                const auto end_ptr = end();

                for(auto ptr = begin(); ptr != end_ptr; ++ptr)
                    allocator_traits::destroy(alloc, ptr);
            }

            template<typename T>
            constexpr void destroy(allocator_type& alloc, const size_type count) const noexcept
            {
                const auto& span = as_span<T>();
                const auto end_ptr = span.end();

                for(auto ptr = span.begin(); ptr != end_ptr; ++ptr)
                    allocator_traits::destroy(alloc, ptr);
            }

            template<::std::invocable<const allocation&, const allocation&> Copy>
            static constexpr allocation
                construct(allocator_type& alloc, const allocation& other, Copy&& copy = copy_as<>)
            {
                const auto& res = get_allocation(alloc, other.size());
                ::std::invoke(::std::forward<Copy>(copy), res, other);
                return res;
            }

            static constexpr allocation&&
                construct(const allocator_type&, allocation&& other) noexcept
            {
                return other;
            }

            [[nodiscard]] constexpr operator bool() const noexcept { return ptr_ != nullptr; }

            template<typename T>
            [[nodiscard]] constexpr ::std::span<T> as_span(const size_type count) const noexcept
            {
                return {pointer_cast<T>(ptr_), count};
            }

            template<typename T>
            [[nodiscard]] constexpr ::std::span<T> as_span() const noexcept
            {
                return as_span<T>(size_ * sizeof(value_type) / sizeof(T));
            }
        };

    private:
        template<bool IsCopy>
        using other_allocator_type =
            ::std::conditional_t<IsCopy, const allocator_type&, allocator_type>;

        template<typename Op, bool IsCopy>
        static constexpr auto assign_op_invocable = //
            ::std::invocable<
                Op,
                constant<before_assign>,
                allocator_type&,
                other_allocator_type<IsCopy>> &&
            ::std::invocable<
                Op,
                constant<after_assign>,
                allocator_type&,
                other_allocator_type<IsCopy>>;

        template<typename Op, bool IsCopy>
        static constexpr auto nothrow_assign_op_invocable = //
            nothrow_invocable<
                Op,
                constant<before_assign>,
                allocator_type&,
                other_allocator_type<IsCopy>> &&
            nothrow_invocable<
                Op,
                constant<after_assign>,
                allocator_type&,
                other_allocator_type<IsCopy>>;

    public:
        static constexpr struct assign_fn
        {
            template<typename Operation>
                requires assign_op_invocable<Operation, false> &&
                propagate_on_container_move_assignment::value
            constexpr void operator()(
                allocator_type& left,
                allocator_type&& right,
                Operation op //
            ) const noexcept(nothrow_assign_op_invocable<Operation, false>)
            {
                ::std::invoke(op, constant<before_assign>{}, left, ::std::move(right));
                left = ::std::move(right);
                ::std::invoke(op, constant<after_assign>{}, left, ::std::move(right));
            }

            template<::std::invocable<allocator_type&, allocator_type> Operation>
                requires(!propagate_on_container_move_assignment::value)
            constexpr void operator()(
                allocator_type& left,
                allocator_type&& right,
                Operation&& op //
            ) const noexcept(nothrow_invocable<Operation, allocator_type&, allocator_type>)
            {
                ::std::invoke(::std::forward<Operation>(op), left, ::std::move(right));
            }

            template<typename Operation>
                requires assign_op_invocable<Operation, true> &&
                propagate_on_container_copy_assignment::value
            constexpr void
                operator()(allocator_type& left, const allocator_type& right, Operation op) const
                noexcept(nothrow_assign_op_invocable<Operation, true>)
            {
                ::std::invoke(op, constant<before_assign>{}, left, right);
                left = right;
                ::std::invoke(op, constant<after_assign>{}, left, right);
            }

            template<::std::invocable<allocator_type&, const allocator_type&> Operation>
                requires(!propagate_on_container_copy_assignment::value)
            constexpr void
                operator()(allocator_type& left, const allocator_type& right, Operation&& op) const
                noexcept(nothrow_invocable<Operation, allocator_type&, const allocator_type&>)
            {
                ::std::invoke(::std::forward<Operation>(op), left, right);
            }
        } assign;

        template<typename... Args>
        static constexpr auto assign_req = //
            ::std::invocable<assign_fn, allocator_type&, Args...> ?
            nothrow_invocable<assign_fn, allocator_type&, Args...> ? //
                expr_req::no_exception :
                expr_req::well_formed :
            expr_req::ill_formed;

        static constexpr struct swap_fn
        {
            template<
                ::std::invocable<constant<before_swap>, allocator_type&, allocator_type&> Operation>
                requires ::std::invocable<
                             Operation,
                             constant<after_swap>,
                             allocator_type&,
                             allocator_type&> &&
                propagate_on_container_swap::value
            constexpr void operator()(
                allocator_type& left,
                allocator_type& right,
                Operation op
            ) const noexcept( //
                nothrow_invocable<
                    Operation,
                    constant<before_swap>,
                    allocator_type&,
                    allocator_type // clang-format off
                    >&& // clang-format on
                    nothrow_invocable<
                        Operation,
                        constant<after_swap>,
                        allocator_type&,
                        allocator_type& // clang-format off
                        > // clang-format on
            )
            {
                ::std::invoke(op, constant<before_swap>{}, left, right);
                ::std::ranges::swap(left, right);
                ::std::invoke(op, constant<after_swap>{}, left, right);
            }

            template<::std::invocable<allocator_type&, allocator_type&> Operation>
                requires(!propagate_on_container_swap::value)
            constexpr void
                operator()(allocator_type& left, allocator_type& right, Operation&& op) const
                noexcept(nothrow_invocable<Operation, allocator_type&, allocator_type&>)
            {
                ::std::invoke(::std::forward<Operation>(op), left, right);
            }
        } swap{};

        template<typename... Args>
        static constexpr auto swap_req = //
            ::std::invocable<swap_fn, allocator_type&, Args...> ?
            nothrow_invocable<swap_fn, allocator_type&, Args...> ? //
                expr_req::no_exception :
                expr_req::well_formed :
            expr_req::ill_formed;

        static constexpr decltype(auto) copy_construct(const allocator_type& alloc) noexcept
        {
            return select_on_container_copy_construction(alloc);
        }

        static constexpr allocation get_allocation(
            allocator_type& alloc,
            const size_type size,
            const const_void_pointer hint = nullptr
        )
        {
            return {size > 0 ? allocate(alloc, size, hint) : nullptr, size};
        }

        static constexpr allocation try_get_allocation(
            allocator_type& alloc,
            const size_type size,
            const const_void_pointer hint = nullptr
        ) noexcept
        {
            return {size > 0 ? try_allocate(alloc, size, hint) : nullptr, size};
        }
    };

    template<
        allocator_req Alloc,
        sequence_container Allocations =
            ::std::vector<typename allocator_traits<Alloc>::allocation> // clang-format off
        > // clang-format on
        requires requires(
            Allocations allocations,
            typename Allocations::value_type value,
            typename allocator_traits<Alloc>::allocation allocation
        ) { requires ::std::same_as<decltype(value), decltype(allocation)>; }
    class basic_allocator_aware
    {
    public:
        using traits = allocator_traits<Alloc>;
        using allocator_type = Alloc;
        using allocations_type = Allocations;
        using allocation = typename allocations_type::value_type;
        using size_type = typename traits::size_type;

    private:
        using iter = typename allocations_type::iterator;
        using const_iter = typename allocations_type::const_iterator;
        using const_alloc_ref = const allocator_type&;
        using this_t = basic_allocator_aware;

        allocator_type allocator_{};
        allocations_type allocations_{};

    public:
        basic_allocator_aware() = default;

        constexpr basic_allocator_aware(const Alloc& alloc) noexcept: allocator_(alloc) {}

        constexpr basic_allocator_aware(const Alloc& alloc) //
            noexcept(nothrow_constructible_from<allocations_type, allocator_type>)
            requires ::std::constructible_from<allocations_type, allocator_type>
            : allocator_(alloc), allocations_(make_obj_uses_allocator<allocations_type>(alloc))
        {
        }

        // TODO: implement

        constexpr basic_allocator_aware(const this_t& other, const Alloc& alloc) //
            noexcept(noexcept(copy_from(other)))
            requires requires { copy_from(other); }
            : allocator_(alloc)
        {
        }

        constexpr basic_allocator_aware(this_t&& other, const Alloc& alloc) //
            noexcept(noexcept(move_from(other)))
            requires requires { move_from(other); }
            : allocator_(alloc)
        {
        }

        constexpr basic_allocator_aware(const this_t& other) //
            noexcept(nothrow_constructible_from<this_t, decltype(other), const_alloc_ref>)
            requires ::std::constructible_from<this_t, decltype(other), const_alloc_ref>
            : this_t(other, traits::copy_construct(other.allocator_))
        {
        }

        basic_allocator_aware(this_t&& other) noexcept = default;

        basic_allocator_aware& operator=(const basic_allocator_aware& other)
        {
            if(this == &other) return;

            destroy();

            allocations_.copy_from(allocator_, other.allocations_);

            return *this;
        }

        basic_allocator_aware& operator=(this_t&& other) //
            noexcept(noexcept(allocations_.move_assign(allocator_, other.allocations_)))
        {
            if(this == &other) return;

            destroy();

            if(allocator_ == other.allocator_) allocations_ = other.allocations_;
            else allocations_.move_from(allocator_, other.allocations_);

            return *this;
        }

        constexpr ~basic_allocator_aware() { deallocate(); }

        template<::std::invocable<allocations_type&, allocation> Func = actions::emplace_back_fn>
        [[nodiscard]] constexpr const auto& allocate(const size_type size, Func&& func = {})
        {
            ::std::invoke(func, allocations_, traits::get_allocation(allocator_, size));
            return ::ranges::back(allocations_);
        }

        [[nodiscard]] constexpr const auto& reallocate(const const_iter iter, const size_type size)
        {
            if constexpr(is_debug)
                if(::std::ranges::distance(iter, allocations_.cbegin()) >= allocations_.size())
                    throw std::out_of_range{"iterator out of range"};

            auto& value = const_cast<allocation&>(*iter); // NOLINT(*-const-cast)
            value.deallocate(allocator_);
            value = traits::get_allocation(allocator_, size);
        }

        constexpr void deallocate(const const_iter iter) noexcept
        {
            iter->deallocate(allocator_);
            actions::cpo::erase(allocations_, iter);
        }

        constexpr void deallocate(const const_iter begin, const const_iter end) noexcept
        {
            for(auto it = begin; it != end; ++it) it->deallocate(allocator_);

            actions::cpo::erase(begin, end);
        }

        constexpr void deallocate() noexcept
        {
            deallocate(allocations_.cbegin(), allocations_.cend());
            allocations_.clear();
        }

        [[nodiscard]] constexpr auto& allocator() const noexcept { return allocator_; }

        [[nodiscard]] constexpr auto& allocator() noexcept { return allocator_; }

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return ::std::ranges::any_of(allocations_);
        }

        [[nodiscard]] constexpr auto size() const noexcept
        {
            return ::std::accumulate(
                allocations_.cbegin(),
                allocations_.cend(),
                [](const allocation& v) noexcept { return v.size; }
            );
        }

        [[nodiscard]] constexpr auto has_value() const noexcept { return allocations_.has_value(); }

        [[nodiscard]] constexpr auto& allocations() const noexcept { return allocations_; }
    };
}