#pragma once

#include <algorithm>

#include "../cassert/cassert.h"
#include "../containers/concepts.h"
#include "pointer_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc, container Range>
        requires requires(typename allocator_traits<Alloc>::allocated allocated) //
    {
        requires ::std::same_as<typename Range::value_type, decltype(allocated)>;
        requires ::std::ranges::output_range<Range, decltype(allocated)>;
    }
    class allocation
    {
        Alloc alloc_{};
        Range allocated_{};

        using traits = allocator_traits<Alloc>;
        using pointer = typename traits::pointer;
        using size_type = typename traits::size_type;
        using allocated_type = typename traits::allocated;

        constexpr void allocate_from(const Range& other_allocated)
        {
            for(auto it = allocated_.begin(); const allocated_type allocated : other_allocated)
            {
                *it = {
                    allocated.ptr == nullptr ? //
                        nullptr :
                        traits::allocate(alloc_, allocated.size),
                    allocated.size //
                };
                ++it;
            }
        }

        constexpr void reallocate_from(const Range& allocated)
        {
            for(auto it = allocated_.begin(); const allocated_type allocated : allocated)
            {
                if(allocated.ptr != nullptr) allocate(it, allocated.size);
                ++it;
            }
        }

        constexpr void verify_rng_iter(const const_iterator_t<Range> it) const
        {
            precondition<::std::invalid_argument>(
                [it, &allocated = allocated_]
                {
                    const auto begin = allocated.cbegin();
                    const auto end = allocated.cend();

                    if(::std::is_constant_evaluated())
                    {
                        for(auto allocated_it = begin; allocated_it != end; ++allocated_it)
                            if(allocated_it == it) return true;
                        return false;
                    }

                    return it >= begin && it <= end;
                },
                "iterator not compatible with range"
            );
        }

        constexpr void deallocate_impl(const const_iterator_t<Range> it)
        {
            if(it->ptr == nullptr) return;

            traits::deallocate(alloc_, it->ptr, it->size);
            const_cast<allocated_type&>(*it) = {}; // NOLINT(*-const-cast)
        }

        constexpr void
            deallocate_impl(const const_iterator_t<Range> begin, const_iterator_t<Range> end)
        {
            for(; begin != end; ++begin) deallocate_impl(begin);
        }

    public:
        using allocator_type = Alloc;
        using range_type = Range;

        allocation() = default;

        template<typename... AllocArgs, typename... RngArgs>
            requires ::std::constructible_from<Alloc, AllocArgs...> &&
                         ::std::constructible_from<Range, RngArgs...>
        constexpr allocation(AllocArgs&&... alloc_args, const empty_t, RngArgs&&... rng_args) //
            noexcept( //
                nothrow_constructible_from<Alloc, AllocArgs...>&&
                    nothrow_constructible_from<Range, RngArgs...>
            ):
            alloc_(::std::forward<AllocArgs>(alloc_args)...),
            allocated_(::std::forward<RngArgs>(rng_args)...)
        {
        }

        constexpr allocation(const allocation& other):
            alloc_(traits::select_on_container_copy_construction(other.alloc_))
        {
            allocate_from(other.allocated_);
        }

        constexpr allocation(allocation&& other) noexcept:
            alloc_(::std::move(other.alloc_)), allocated_(other.allocated_)
        {
        }

        constexpr allocation& operator=(const allocation& other)
        {
            if(this == &other) return *this;

            if constexpr(traits::propagate_on_container_copy_assignment::value)
            {
                if(alloc_ != other.alloc_) release();
                alloc_ = other.alloc_;
                allocate_from(other.allocated_);
            }
            else reallocate_from(other.allocated_);

            return *this;
        }

        constexpr allocation& operator=(allocation&& other) noexcept
        {
            if(this == &other) return *this;

            if constexpr(traits::propagate_on_container_move_assignment::value)
            {
                release();
                alloc_ = ::std::move(other.alloc_);
            }
            else if(alloc_ != other.alloc_)
            {
                reallocate_from(other.allocated_);
                return *this;
            }

            ::std::ranges::copy(other.allocated_, allocated_.begin());

            return *this;
        }

        constexpr ~allocation() noexcept { release(); }

        constexpr auto rng() const noexcept
        {
            namespace std_rng = ::std::ranges;
            return std_rng::subrange{std_rng::cbegin(allocated_), std_rng::cend(allocated_)};
        }

        constexpr void allocate(const const_iterator_t<Range> it, const size_type size)
        {
            verify_rng_iter(it);

            precondition<::std::invalid_argument>(
                [it, end = allocated_.cend()] { return it != end; },
                "input iterator cannot be the end iterator"
            );

            if(size <= it->size) return;

            if(it->ptr != nullptr) traits::deallocate(alloc_, it->ptr, it->size);

            const_cast<allocated_type&>(*it) = // NOLINT(*-const-cast)
                {traits::allocate(alloc_, size), size};
        }

        constexpr void deallocate(const const_iterator_t<Range> it)
        {
            verify_rng_iter(it);
            deallocate_impl(it);
        }

        constexpr void deallocate(const const_iterator_t<Range> begin, const_iterator_t<Range> end)
        {
            verify_rng_iter(begin);
            verify_rng_iter(end);
            deallocate_impl(begin, end);
        }

        constexpr void release() noexcept { deallocate_impl(allocated_.begin(), allocated_.end()); }
    };

    namespace details
    {
        template<
            typename Alloc,
            typename AllocTraits = allocator_traits<Alloc>,
            typename Base =
                allocation<Alloc, ::std::array<typename allocator_traits<Alloc>::allocated, 1>>>
        class object_allocation : Base
        {
            using pointer = typename AllocTraits::pointer;
            using const_pointer = typename AllocTraits::const_pointer;

            constexpr void allocate(const typename AllocTraits::size_type size)
            {
                Base::allocate(get_iter(), size);
            }

            constexpr void deallocate() noexcept { Base::deallocate(get_iter()); }

            [[nodiscard]] constexpr auto& get_raw_ptr() const noexcept { return allocated().ptr; }

            template<typename T>
            [[nodiscard]] constexpr T* get_ptr() const noexcept
            {
                return to_other_address<T>(pointer_traits<pointer>::to_address(get_raw_ptr()));
            }

            [[nodiscard]] constexpr auto get_size() const noexcept { return allocated().size; }

            [[nodiscard]] constexpr auto& allocated() const noexcept { return *get_iter(); }

            [[nodiscard]] constexpr auto get_iter() const noexcept { return Base::rng().begin(); }

            struct traits_base
            {
                ::std::string_view curent_type{};
                void (*destroy)(const pointer&) noexcept = nullptr;
                void (*move_construct)(const pointer&, const pointer&) = nullptr;
                void (*copy_construct)(const pointer&, const const_pointer&) = nullptr;
                void (*move_assign)(const pointer&, const pointer&) = nullptr;
                void (*copy_assign)(const pointer&, const pointer&) = nullptr;
            };

            template<typename T>
            constexpr void set_traits() noexcept
            {
                traits_ = &traits_v<T>;
            }

            constexpr void destroy() noexcept { (*(traits_->destroy))(get_raw_ptr()); }

            template<typename T>
            struct traits : traits_base
            {
                constexpr traits() noexcept:
                    traits_base{
                        type_id<T>,
                        &destroy_impl,
                        &move_construct_impl,
                        &copy_construct_impl,
                        &move_assign_impl,
                        &copy_assign_impl //
                    }
                {
                }

                [[nodiscard]] static constexpr T& get(const pointer& p) noexcept
                {
                    return to_other_address<T>(pointer_traits<pointer>::to_address(p));
                }

                [[nodiscard]] static constexpr const T& get(const const_pointer& p) noexcept
                {
                    return to_other_address<T>(pointer_traits<pointer>::to_address(p));
                }

                static constexpr void destroy_impl(const pointer& p) noexcept
                {
                    if(p == nullptr) return;

                    ::std::destroy_at(get(p));
                }

                static constexpr void
                    move_construct_impl(const pointer& this_, const pointer& other)
                {
                    ::new(this_) T(::std::move(get(other)));
                }

                static constexpr void
                    copy_construct_impl(const pointer& this_, const const_pointer& other)
                {
                    ::new(this_) T(get(other));
                }

                static constexpr void move_assign_impl(const pointer& this_, const pointer& other)
                {
                    get(this_) = ::std::move(get(other));
                }

                static constexpr void copy_assign_impl(const pointer& this_, const pointer& other)
                {
                    get(this_) = get(other);
                }
            };

        public:
            object_allocation() = default;

            template<typename T>
            [[nodiscard]] constexpr bool is_same_type() const noexcept
            {
                return type_id<T> == type();
            }

            [[nodiscard]] constexpr auto type() const noexcept
            {
                return traits_ == nullptr ? type_id<void> : traits_->curent_type;
            }

            template<typename T>
            [[nodiscard]] constexpr T& get() const noexcept
            {
                return *get_ptr<T>();
            }

            template<::std::copy_constructible T, typename... U>
                requires ::std::constructible_from<Base, U..., empty_t>
            constexpr object_allocation(T&& t, U&&... u) //
                noexcept(nothrow_constructible_from<Base, U..., empty_t>):
                Base(::std::forward<U>(u)..., empty)
            {
                emplace(::std::forward<T>(t));
            }

            constexpr object_allocation(const object_allocation& other):
                Base(other), traits_(other.traits_)
            {
                if(other.has_value())
                    (*(other.traits_->copy_construct))(get_raw_ptr(), other.get_raw_ptr());
            }

            constexpr object_allocation(object_allocation&& other) noexcept:
                Base(static_cast<Base&&>(other)), traits_(other.traits_)
            {
            }

            constexpr object_allocation& operator=(const object_allocation& other)
            {
                if(this == &other) return *this;

                if(type() == other.type())
                {
                    static_cast<Base&>(*this) = static_cast<const Base&>(other);
                }

                destroy();

                (*(other.traits_->copy_assign))(get_raw_ptr(), other.get_raw_ptr());
                return *this;
            }

            constexpr object_allocation& operator=(object_allocation&& other) //
                noexcept(noexcept(other.traits_->move_assign(get_raw_ptr(), other.get_raw_ptr())))
            {
                if(this == &other) return *this;

                static_cast<Base&>(*this) = static_cast<Base&&>(other);
                other.traits_->move_assign(*this, other);
                return *this;
            }

            constexpr ~object_allocation() noexcept { reset(); }

            template<::std::copy_constructible T, typename... Args>
                requires ::std::constructible_from<T, Args...>
            constexpr void emplace(Args&&... args) //
                noexcept(noexcept(emplace_impl<T>(::std::forward<Args>(args)...)))
            {
                emplace_impl<T>(::std::forward<Args>(args)...);
            }

            constexpr void reset() noexcept
            {
                destroy();
                deallocate();
            }

            [[nodiscard]] constexpr bool has_value() const noexcept { return traits_ != nullptr; }

        private:
            template<typename T>
            static constexpr traits<T> traits_v{};

            const traits_base* traits_{};
        };
    }

    using object_allocation = details::object_allocation;
}