#pragma once

#include "pointer_traits.h"
#include "allocator_traits.h"
#include "../type_traits/object.h"

namespace stdsharp
{
    namespace details
    {
        template<typename Alloc>
        struct basic_object_allocation
        {
            constexpr auto get_allocator() const noexcept { return allocator; }

        private:
            using alloc_traits = allocator_traits<Alloc>;
            using pointer = typename alloc_traits::pointer;
            using const_pointer = typename alloc_traits::const_pointer;
            using size_type = typename alloc_traits::size_type;

            Alloc allocator{};
            typename alloc_traits::allocated allocated_{};

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

                alloc_traits::deallocate(this->get_allocator(), get_raw_ptr(), allocated_.size);
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
                            alloc_traits::allocate(allocator, size),
                            size //
                        };
                    }
                }
                else allocated_ = {alloc_traits::allocate(allocator, size), size};
            }

            constexpr void
                assign_or_construct(const auto assign_fn, const auto construct_fn, auto&& other)
            {
                if(!other.has_value())
                {
                    if(has_value()) reset();
                    return;
                }

                if(type() == other.type()) (*assign_fn)(get_raw_ptr(), other.get_raw_ptr());
                else
                {
                    reserve(other.allocated_.size);

                    (*construct_fn)(get_raw_ptr(), other.get_raw_ptr());
                    traits_ = other.traits_;
                }
            }

            using assign_op = allocator_assign_operation;

        public:
            template<typename... Args>
                requires ::std::constructible_from<Alloc, Args...>
            constexpr basic_object_allocation(Args&&... args) //
                noexcept(nothrow_constructible_from<Alloc, Args...>):
                allocator(::std::forward<Args>(args)...)
            {
            }

            template<typename T, typename... Args, ::std::movable ValueType = ::std::decay_t<T>>
                requires ::std::constructible_from<Alloc, Args...> &&
                             ::std::constructible_from<ValueType, T>
            constexpr basic_object_allocation(T&& value, Args&&... args):
                allocator(::std::forward<Args>(args)...),
                allocated_{
                    {alloc_traits::allocate(allocator, sizeof(ValueType)), sizeof(ValueType)} //
                },
                traits_(&traits_v<ValueType>)
            {
                ::new(get_raw_ptr()) ValueType{::std::forward<ValueType>(value)};
            }

            basic_object_allocation() = default;

            constexpr basic_object_allocation(const basic_object_allocation& other):
                allocator(alloc_traits::copy_construct(other.allocator)),
                allocated_( //
                    {
                        other.has_value() ? alloc_traits::allocate(allocator, other.size) : nullptr,
                        other.size //
                    }
                ),
                traits_(other.traits_)
            {
                if(other.has_value())
                    (*(other.traits_->copy_construct))(get_raw_ptr(), other.get_raw_ptr());
            }

            constexpr basic_object_allocation(basic_object_allocation&& other) noexcept:
                allocator(::std::move(other.allocator)),
                allocated_(::std::move(other.allocated_)),
                traits_(::std::exchange(other.traits_, nullptr))
            {
            }

            constexpr basic_object_allocation& operator=(const basic_object_allocation& other)
            {
                if(this == &other) return *this;

                constexpr auto copy_assign = [this](const basic_object_allocation& other)
                {
                    assign_or_construct(
                        other.traits_->copy_assign,
                        other.traits_->copy_construct,
                        other
                    ); //
                };

                alloc_traits::assign(
                    allocator,
                    other.allocator,
                    make_trivial_invocables(
                        [this](const Alloc& left, const Alloc& right, const constant<assign_op::before>)
                        {
                            if(left != right) reset();
                        },
                        [&other,
                         &copy_assign](const Alloc&, const Alloc&, const constant<assign_op::after>)
                        {
                            copy_assign(other); //
                        },
                        [&other, &copy_assign](const Alloc&, const Alloc&) { copy_assign(other); }
                    )
                );

                return *this;
            }

            constexpr basic_object_allocation& operator=(basic_object_allocation&& other) noexcept
            {
                if(this == &other) return *this;

                alloc_traits::assign(
                    allocator,
                    ::std::move(other.allocator),
                    make_trivial_invocables(
                        [this](const Alloc&, const Alloc&, const constant<assign_op::before>)
                        {
                            reset(); //
                        },
                        [this, &other](const Alloc&, const Alloc&, const constant<assign_op::after>)
                        {
                            allocated_ = other.allocated_;
                            traits_ = other.traits_;
                        },
                        [this, &other](const Alloc&, const Alloc&)
                        {
                            assign_or_construct(
                                other.traits_->move_assign,
                                other.traits_->move_construct,
                                other
                            ); //
                        }
                    )
                );

                return *this;
            }

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

            template<::std::movable T, typename... Args>
                requires ::std::constructible_from<T, Args...>
            constexpr void emplace(Args&&... args)
            {
                if(type_id<T>() == type()) get<T>() = T{::std::forward<Args>(args)...};
                else
                {
                    reserve(sizeof(T));

                    traits_ = &traits_v<T>;
                    new(get_raw_ptr()) T{::std::forward<Args>(args)...};
                }
            }
        };
    }

    template<typename Alloc>
    class object_allocation : details::basic_object_allocation<Alloc>
    {
        using m_base = details::basic_object_allocation<Alloc>;

    public:
        using m_base::get;
        using m_base::has_value;
        using m_base::is_same_type;
        using m_base::operator bool;
        using m_base::reset;
        using m_base::reserved;
        using m_base::size;
        using m_base::type;
        using m_base::get_allocator;

        template<typename... Args>
            requires ::std::constructible_from<m_base, Args...>
        constexpr object_allocation(Args&&... args) //
            noexcept(nothrow_constructible_from<Alloc, Args...>):
            m_base(::std::forward<Args>(args)...)
        {
        }

        template<typename T, typename... Args, ::std::copyable ValueType = ::std::decay_t<T>>
            requires ::std::constructible_from<Alloc, Args...> &&
            ::std::constructible_from<ValueType, T>
        constexpr object_allocation(T&& value, Args&&... args):
            m_base(::std::forward<T>(value), ::std::forward<Args>(args)...)
        {
        }

        object_allocation() = default;

        template<::std::copyable T, typename... Args>
            requires requires //
        {
            details::basic_object_allocation<Alloc>::emplace(::std::declval<Args>()...); //
        }
        constexpr void emplace(Args&&... args)
        {
            details::basic_object_allocation<Alloc>::emplace(::std::forward<Args>(args)...);
        }
    };

    template<typename Alloc>
    class unique_object_allocation : details::basic_object_allocation<Alloc>, public unique_object
    {
        using m_base = details::basic_object_allocation<Alloc>;

    public:
        using m_base::get;
        using m_base::has_value;
        using m_base::is_same_type;
        using m_base::operator bool;
        using m_base::reset;
        using m_base::reserved;
        using m_base::size;
        using m_base::type;
        using m_base::get_allocator;
    };
}