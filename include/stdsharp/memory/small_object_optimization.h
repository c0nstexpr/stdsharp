#pragma once

#include "composed_allocator.h"
#include "static_allocator.h"
#include "../cmath/cmath.h"

namespace stdsharp
{
    namespace details
    {
        struct alignas(::std::max_align_t) soo_storage
        {
        private:
            template<typename T> // TODO: make it real constexpr
            [[nodiscard]] friend constexpr auto to_other_address(soo_storage* t) noexcept
            {
                return static_cast<T*>(static_cast<void*>(t));
            }

            template<typename T> // TODO: make it real constexpr
            [[nodiscard]] friend constexpr auto to_other_address(const soo_storage* t) noexcept
            {
                return static_cast<const T*>(static_cast<const void*>(t));
            }
        };

        template<typename FallbackAlloc>
        class small_object
        {
            using static_alloc = static_allocator<soo_storage, 1>;
            using fallback_traits = allocator_traits<FallbackAlloc>;
            using alloc_type = composed_allocator<static_allocator<soo_storage, 1>, FallbackAlloc>;

            alloc_type alloc_;
            typename alloc_type::alloc_ret allocated_{};
            ::std::uintmax_t size_{};

            ::std::string_view curent_type_{};

            void (*destroy_)(small_object&) = nullptr;
            void (*move_to_)(small_object&, small_object&) = nullptr;
            void (*copy_to_)(small_object&, small_object&) = nullptr;

            constexpr void allocate(::std::uintmax_t size)
            {
                size_ = size;
                allocated_ = alloc_.allocate(size_);
            }

            constexpr void deallocate() noexcept { alloc_.deallocate(allocated_, size_); }

            template<typename T>
            constexpr T* get_ptr() const noexcept
            {
                return to_other_address<T>(allocated_.ptr);
            }

            template<typename T>
            constexpr T& get() const noexcept
            {
                return *get_ptr<T>();
            }

            template<typename T>
            static constexpr void destroy_impl(small_object& this_)
            {
                this_.alloc_.destroy(this_.allocated_.index, this_.get_ptr<::std::decay_t<T>>());
            }

            template<typename T, typename Proj = ::std::identity>
            static constexpr void
                construct_to_impl(small_object& this_, small_object& other, Proj&& proj = {})
            {
                other.emplace<T>(::std::invoke(proj, this_.get<T>()));
            }

            template<typename T>
            static constexpr void move_to_impl(small_object& this_, small_object& other)
            {
                construct_to_impl<T>(this_, other, [](T&& t) noexcept { return ::std::move(t); });
            }

            template<typename T>
            static constexpr void copy_to_impl(small_object& this_, small_object& other)
            {
                construct_to_impl<T>(this_, other);
            }

            template<move_assignable T, typename... Args>
                requires ::std::invocable<construct_fn<T>, Args...>
            static constexpr void construct_at_impl(
                small_object& this_,
                Args&&... args
            ) noexcept(nothrow_invocable<construct_fn<T>, Args...>&& nothrow_move_assignable<T>)
            {
                this_.get<T>() = construct<T>(::std::forward<Args>(args)...);
            }

            constexpr auto& get_sbo() noexcept { return get<0>(alloc_.allocators); }

            constexpr auto& get_fallback_alloc() noexcept { return get<1>(alloc_.allocators); }

        public:
            small_object() = default;

            template<typename... T>
            constexpr small_object(T&&... t):
                alloc_{.allocators = {static_alloc{}, ::std::forward<T>(t)...}}
            {
            }

            constexpr small_object(const small_object& other):
                alloc_{
                    .allocators =
                        {static_alloc{},
                         fallback_traits:: //
                         select_on_container_copy_construction((other.get_fallback_alloc()))} //
                }
            {
            }

            template<typename T, typename... U>
                requires ::std::constructible_from<T, U...>
            constexpr void emplace(U&&... u)
            {
                if(type_id<T> == curent_type_)
                {
                    construct_at_impl<T>(*this, ::std::forward<U>(u)...);
                    return;
                }

                reset();

                {
                    constexpr auto n = ceil_reminder(sizeof(T), sizeof(soo_storage));

                    if(size_ < n) allocate(n);
                }

                alloc_.construct_at(get_ptr<T>(allocated_.ptr), ::std::forward<U>(u)...);

                destroy_ = &destroy_impl<T>;
                move_to_ = &move_to_impl<T>;
                copy_to_ = &copy_to_impl<T>;
                curent_type_ = type_id<T>;
            }

            constexpr void reset()
            {
                if(!has_value()) return;

                (*destroy_)(*this);
                destroy_ = nullptr;
            }

            [[nodiscard]] constexpr bool has_value() const noexcept { return destroy_ != nullptr; }
        };
    }

    template<typename FallbackAlloc>
    class small_object :
        private details::small_object<
            typename allocator_traits<FallbackAlloc>::template rebind_alloc<details::soo_storage>>
    {
        using base_type = details::small_object<FallbackAlloc>;
    };
}