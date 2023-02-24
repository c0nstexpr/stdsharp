#pragma once

#include "pointer_traits.h"
#include "allocator_traits.h"

namespace stdsharp
{
    namespace details
    {
        template<typename Alloc>
        struct basic_object_allocation
        {
            constexpr auto get_allocator() const noexcept { return allocator; }

            using AllocTraits = allocator_traits<Alloc>;

        private:
            using pointer = typename AllocTraits::pointer;
            using const_pointer = typename AllocTraits::const_pointer;
            using size_type = typename AllocTraits::size_type;

            Alloc allocator{};
            typename AllocTraits::allocated allocated_{};

            struct traits_base
            {
                ::std::string_view curent_type{};
                ::std::size_t type_size{};
                void (*destroy)(const pointer&) noexcept = nullptr;
                void (*move_construct)(const pointer&, const pointer&) = nullptr;
                void (*copy_construct)(const pointer&, const const_pointer&) = nullptr;
                void (*move_assign)(const pointer&, const pointer&) = nullptr;
                void (*copy_assign)(const pointer&, const pointer&) = nullptr;
            };

            const traits_base* traits_{};

            template<typename T>
            struct movable_traits
            {
                [[nodiscard]] static constexpr T& get(const pointer& p) noexcept
                {
                    return point_as<T>(pointer_traits<pointer>::to_address(p));
                }

                [[nodiscard]] static constexpr const T& get(const const_pointer& p) noexcept
                {
                    return point_as<T>(pointer_traits<pointer>::to_address(p));
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

                static constexpr void move_assign_impl(const pointer& this_, const pointer& other)
                {
                    get(this_) = ::std::move(get(other));
                }
            };

            template<typename T>
            struct traits;

            template<::std::movable T>
            struct traits<T> : movable_traits<T>
            {
                constexpr traits() noexcept:
                    movable_traits<T>{
                        type_id<T>,
                        sizeof(T),
                        &movable_traits<T>::destroy_impl,
                        &movable_traits<T>::move_construct_impl,
                        nullptr,
                        &movable_traits<T>::move_assign_impl,
                        nullptr //
                    }
                {
                }
            };

            template<::std::copyable T>
            struct traits<T> : traits_base
            {
                constexpr traits() noexcept:
                    traits_base{
                        type_id<T>,
                        sizeof(T),
                        &movable_traits<T>::destroy_impl,
                        &movable_traits<T>::move_construct_impl,
                        &copy_construct_impl,
                        &movable_traits<T>::move_assign_impl,
                        &copy_assign_impl //
                    }
                {
                }

                static constexpr void
                    copy_construct_impl(const pointer& this_, const const_pointer& other)
                {
                    ::new(this_) T(movable_traits<T>::get(other));
                }

                static constexpr void copy_assign_impl(const pointer& this_, const pointer& other)
                {
                    movable_traits<T>::get(this_) = movable_traits<T>::get(other);
                }
            };

            template<typename T>
            static constexpr traits<T> traits_v{};

            [[nodiscard]] constexpr auto& get_raw_ptr() const noexcept { return allocated_.ptr; }

            constexpr void destroy() const noexcept
            {
                if(traits_ != nullptr)
                    (*(::std::exchange(traits_, nullptr)->destroy))(get_raw_ptr());
            }

            constexpr void deallocate() noexcept
            {
                if(allocated_.ptr == nullptr) return;

                AllocTraits::deallocate(this->get_allocator(), get_raw_ptr(), allocated_.size);
                allocated_ = {};
            }

            constexpr void reserve(const size_type size)
            {
                if(has_value())
                {
                    destroy();

                    if(allocated_.size < size)
                    {
                        deallocate();
                        allocated_ = {
                            AllocTraits::allocate(this->get_allocator(), size),
                            size //
                        };
                    }
                }
                else allocated_ = {AllocTraits::allocate(this->get_allocator(), size), size};
            }

            constexpr void before_move_assign(basic_object_allocation&& other) noexcept
            {
                if(allocator != other.allocator) reset();
            }

            constexpr void after_move_assign(basic_object_allocation&& other) noexcept
            {
                if(this->has_value()) this->reset();

                allocated_ = other.allocated_;
                traits_ = other.traits_;
            }

            constexpr void move_assign(basic_object_allocation&& other)
            {
                const auto size = other.allocated_.size;

                if(other.has_value())
                {
                    if(type() == other.type())
                        (*(other.traits_->move_assign))(get_raw_ptr(), other.get_raw_ptr());
                    else
                    {
                        reserve(size);

                        (*(other.traits_->move_construct))(get_raw_ptr(), other.get_raw_ptr());
                        traits_ = other.traits_;
                    }
                }
                else if(has_value()) reset();
            }

        public:
            basic_object_allocation(const basic_object_allocation&) = default;
            basic_object_allocation(basic_object_allocation&&) noexcept = default;
            basic_object_allocation& operator=(const basic_object_allocation&) = default;
            basic_object_allocation& operator=(basic_object_allocation&&) noexcept = default;

            constexpr ~basic_object_allocation() noexcept { reset(); }

            template<typename T>
            [[nodiscard]] constexpr bool is_same_type() const noexcept
            {
                return type_id<T> == type();
            }

            [[nodiscard]] constexpr auto type() const noexcept
            {
                return traits_ == nullptr ? type_id<void> : traits_->curent_type;
            }

            [[nodiscard]] constexpr auto size() const noexcept
            {
                return traits_ == nullptr ? 0 : traits_->type_size;
            }

            [[nodiscard]] constexpr auto reserved() const noexcept { return allocated_.size; }

            template<typename T>
            [[nodiscard]] constexpr T& get() noexcept
            {
                return *point_as<T>(pointer_traits<pointer>::to_address(get_raw_ptr()));
            }

            template<typename T>
            [[nodiscard]] constexpr const T& get() const noexcept
            {
                return *point_as<T>(pointer_traits<pointer>::to_address(get_raw_ptr()));
            }

            constexpr void reset() noexcept
            {
                destroy();
                deallocate();
            }

            [[nodiscard]] constexpr bool has_value() const noexcept { return traits_ != nullptr; }

            [[nodiscard]] constexpr explicit operator bool() const noexcept { return has_value(); }
        };

        template<typename Alloc>
        class object_allocation : basic_object_allocation<Alloc>
        {
        public:
            template<::std::copyable T, typename... Args>
                requires requires //
            {
                basic_object_allocation<Alloc>::emplace(::std::declval<Args>()...); //
            }
            constexpr void emplace(Args&&... args)
            {
                basic_object_allocation<Alloc>::emplace(::std::forward<Args>(args)...);
            }
        };
    }

    template<typename Alloc>
    using object_allocation = details::object_allocation<Alloc>;

    template<typename Alloc>
    using unique_object_allocation = details::basic_object_allocation<Alloc>;
}