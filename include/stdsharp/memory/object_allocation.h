#pragma once

#include <utility>

#include "allocation.h"
#include "pointer_traits.h"
#include "stdsharp/type_traits/object.h"

namespace stdsharp
{
    namespace details
    {
        template<
            typename Alloc,
            typename Derived,
            typename Base = allocation<Alloc, Derived>,
            typename AllocTraits = allocator_traits<Alloc>>
        struct base_object_allocation : private Base
        {
            constexpr auto get_allocator() const noexcept { return Base::get_allocator(); }

        private:
            friend Base;

            using pointer = typename AllocTraits::pointer;
            using const_pointer = typename AllocTraits::const_pointer;
            using size_type = typename AllocTraits::size_type;

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

            constexpr void before_move_assign(base_object_allocation&& other) noexcept
            {
                if(Base::get_allocator() != other.get_allocator()) reset();
            }

            constexpr void after_move_assign(base_object_allocation&& other) noexcept
            {
                if(this->has_value()) this->reset();

                allocated_ = other.allocated_;
                traits_ = other.traits_;
            }

            constexpr void move_assign(base_object_allocation&& other)
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
            base_object_allocation(const base_object_allocation&) = default;
            base_object_allocation(base_object_allocation&&) noexcept = default;
            base_object_allocation& operator=(const base_object_allocation&) = default;
            base_object_allocation& operator=(base_object_allocation&&) noexcept = default;

            constexpr ~base_object_allocation() noexcept { reset(); }

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

        protected:
            const traits_base* traits_{};

            typename AllocTraits::allocated allocated_{};
        };

        template<typename Alloc>
        class object_allocation : base_object_allocation<Alloc, object_allocation<Alloc>>
        {
        private:
            using m_base = base_object_allocation<Alloc, object_allocation<Alloc>>;

        public:
            using m_base::get_allocator;
            using m_base::reset;
            using m_base::has_value;
            using m_base::type;

        private:
            using m_base::allocated_;
            using m_base::traits_;
            using m_base::get_raw_ptr;

            constexpr void before_copy_assign(const object_allocation& other) noexcept
            {
                if(this->get_allocator() != other.get_allocator()) reset();
            }

            constexpr void after_copy_assign(const object_allocation& other) { copy_assign(other); }

            constexpr void copy_assign(const object_allocation& other)
            {
                const auto size = other.allocated_.size;

                if(other.has_value())
                {
                    if(type() == other.type())
                        (*(other.traits_->copy_assign))(get_raw_ptr(), other.get_raw_ptr());
                    else
                    {
                        reserve(size);

                        (*(other.traits_->copy_construct))(get_raw_ptr(), other.get_raw_ptr());
                        traits_ = other.traits_;
                    }
                }
                else if(has_value()) reset();
            }

            template<typename T, typename... Args>
            constexpr void emplace_impl(Args&&... args)
            {
                reserve(sizeof(T));
                ::new(pointer_traits<pointer>::to_address(get_raw_ptr()))
                    T{::std::forward<Args>(args)...};
                traits_ = &traits_v<T>;
            }

        public:
            constexpr object_allocation(const object_allocation& other):
                m_base(other), traits_(other.traits_)
            {
                if(other.has_value())
                    (*(other.traits_->copy_construct))(get_raw_ptr(), other.get_raw_ptr());
            }

            template<::std::movable T, typename... U>
                requires ::std::constructible_from<m_base, U...>
            constexpr object_allocation(T&& t, U&&... u) //
                noexcept(nothrow_constructible_from<m_base, U...>):
                m_base(::std::forward<U>(u)...)
            {
                emplace(::std::forward<T>(t));
            }

            object_allocation(object_allocation&&) noexcept = default;
            object_allocation& operator=(const object_allocation&) = default;
            object_allocation&
                operator=(object_allocation&&) = default; // NOLINT(*-noexcept-move-constructor)

            template<::std::movable T, typename... Args>
                requires ::std::constructible_from<T, Args...>
            constexpr void emplace(Args&&... args)
            {
                if constexpr(::std::copyable<T>)
                {
                    if(type() == type_id<T>) get<T>() = T{::std::forward<Args>(args)...};
                    else emplace_impl<T>(::std::forward<Args>(args)...);
                }
                else emplace_impl<T>(::std::forward<Args>(args)...);
            }
        };
    }

    template<typename Alloc>
    using object_allocation = details::object_allocation<Alloc>;

    template<typename Alloc>
    using unique_object_allocation = details::unique_object_allocation<Alloc>;
}