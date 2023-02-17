#pragma once

#include "allocation.h"
#include "pointer_traits.h"

namespace stdsharp
{
    namespace details
    {

        template<
            typename Alloc,
            typename AllocTraits = allocator_traits<Alloc>,
            typename Allocated = typename AllocTraits::allocated>
        class object_allocation :
            allocation<
                Alloc,
                ::std::array<Allocated, 1>,
                object_allocation<Alloc> // clang-format off
            > // clang-format on
        {
            using m_base = allocation<Alloc, ::std::array<Allocated, 1>, object_allocation>;

            using pointer = typename AllocTraits::pointer;
            using const_pointer = typename AllocTraits::const_pointer;

            constexpr void on_deallocate(const Allocated&) noexcept { destroy(); }

            constexpr void allocate(const typename AllocTraits::size_type size)
            {
                m_base::allocate(get_iter(), size);
            }

            [[nodiscard]] constexpr auto& get_raw_ptr() const noexcept { return allocated().ptr; }

            template<typename T>
            [[nodiscard]] constexpr T* get_ptr() const noexcept
            {
                return point_as<T>(pointer_traits<pointer>::to_address(get_raw_ptr()));
            }

            [[nodiscard]] constexpr auto get_size() const noexcept { return allocated().size; }

            [[nodiscard]] constexpr auto& allocated() const noexcept { return *get_iter(); }

            [[nodiscard]] constexpr auto get_iter() const noexcept { return m_base::rng().begin(); }

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

            constexpr void destroy() noexcept
            {
                (*(traits_->destroy))(get_raw_ptr());
                traits_ = nullptr;
            }

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

            template<::std::copy_constructible T, typename... U>
                requires ::std::constructible_from<m_base, U..., empty_t>
            constexpr object_allocation(T&& t, U&&... u) //
                noexcept(nothrow_constructible_from<m_base, U..., empty_t>):
                m_base(::std::forward<U>(u)..., empty)
            {
                emplace(::std::forward<T>(t));
            }

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

            constexpr object_allocation(const object_allocation& other):
                m_base(other), traits_(other.traits_)
            {
                if(other.has_value())
                    (*(other.traits_->copy_construct))(get_raw_ptr(), other.get_raw_ptr());
            }

            constexpr object_allocation(object_allocation&& other) noexcept:
                m_base(static_cast<m_base&&>(other)), traits_(other.traits_)
            {
            }

            constexpr object_allocation& operator=(const object_allocation& other)
            {
                if(this == &other) return *this;

                static_cast<m_base&>(*this) = static_cast<const m_base&>(other);
                if(type() == other.type() && has_value())
                    (*(other.traits_->copy_assign))(get_raw_ptr(), other.get_raw_ptr());
                else (*(other.traits_->copy_construct))(get_raw_ptr(), other.get_raw_ptr());

                return *this;
            }

            constexpr object_allocation& operator=(object_allocation&& other) //
                noexcept(noexcept(other.traits_->move_assign(get_raw_ptr(), other.get_raw_ptr())))
            {
                if(this == &other) return *this;

                static_cast<m_base&>(*this) = static_cast<m_base&&>(other);
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
                m_base::deallocate(get_iter());
            }

            [[nodiscard]] constexpr bool has_value() const noexcept { return traits_ != nullptr; }

        private:
            template<typename T>
            static constexpr traits<T> traits_v{};

            const traits_base* traits_{};
        };
    }

    template<typename Alloc>
    using object_allocation = details::object_allocation<Alloc>;
}