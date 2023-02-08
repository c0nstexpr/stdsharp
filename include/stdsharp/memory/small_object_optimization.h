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

        class small_object
        {
            using static_alloc = static_allocator<soo_storage, 1>;

            constexpr void allocate(::std::uintmax_t size)
            {
                size_ = size;
                allocated_ = alloc_.allocate(size_);
            }

            constexpr void deallocate() noexcept
            {
                alloc_.deallocate(allocated_, size_);
                allocated_ = nullptr;
                size_ = 0;
            }

            template<typename T>
            constexpr T* get_ptr() const noexcept
            {
                return to_other_address<T>(allocated_);
            }

            template<typename T>
            constexpr T& get() const noexcept
            {
                return *get_ptr<T>();
            }

            struct traits_base
            {
                ::std::string_view curent_type{};
                void (*destroy)(small_object&) noexcept = nullptr;
                void (*move_construct)(small_object&, small_object&&) = nullptr;
                void (*copy_construct)(small_object&, const small_object&) = nullptr;
                void (*move_assign)(small_object&, small_object&&) = nullptr;
                void (*copy_assign)(small_object&, const small_object&) = nullptr;
            };

            template<typename T>
            constexpr void set_traits() noexcept
            {
                traits_ = &traits_v<T>;
            }

            template<typename T, typename... Args>
                requires move_assignable<T>
            constexpr void assign_as(Args&&... args) //
                noexcept(nothrow_move_assignable<T>&& nothrow_constructible_from<T, Args...>)
            {
                get<T>() = construct<T>(::std::forward<Args>(args)...);
            }

            template<typename T, typename... Args>
            constexpr bool try_assign_as(Args&&... args) //
                noexcept(noexcept(assign_as<T>(*::std::declval<Args>()...)))
                requires requires { assign_as<T>(*::std::declval<Args>()...); }
            {
                if(!is_same_type<T>()) return false;

                assign_as<T>(::std::forward<Args>(args)...);
                return true;
            }

            template<typename T, typename... Args>
            constexpr bool try_assign_as(Args&&...) noexcept
            {
                return false;
            }

            template<typename T, typename... Args>
            constexpr void overwrite(Args&&... args) //
                noexcept(sizeof(T) <= static_alloc::size && nothrow_constructible_from<T, Args...>)
            {
                destroy();

                {
                    constexpr auto n = ceil_reminder(sizeof(T), sizeof(soo_storage));

                    if(size_ < n) allocate(n);
                }

                ::std::construct_at(get_ptr<T>(), ::std::forward<Args>(args)...);

                set_traits<T>();
            }

            template<typename T, typename... Args>
            constexpr void construct_from(Args&&... args) //
                noexcept(sizeof(T) <= static_alloc::size && nothrow_constructible_from<T, Args...>)
            {
                allocate(ceil_reminder(sizeof(T), sizeof(soo_storage)));
                ::std::construct_at(get_ptr<T>(), ::std::forward<Args>(args)...);
                set_traits<T>();
            }

            template<typename T, typename... Args>
            constexpr void emplace_impl(Args&&... args) noexcept( //
                noexcept(
                    try_assign_as<T>(::std::declval<Args>()...),
                    overwrite<T>(::std::declval<Args>()...)
                )
            )
            {
                if(try_assign_as<T>(::std::forward<Args>(args)...)) return;
                overwrite<T>(::std::forward<Args>(args)...);
            }

            constexpr void destroy() noexcept { (*(traits_->destroy))(*this); }

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

                static constexpr void destroy_impl(small_object& this_)
                {
                    if(!this_.has_value()) return;

                    ::std::destroy_at(this_.get_ptr<T>());
                    this_.traits_ = nullptr;
                }

                static constexpr void move_construct_impl(small_object& this_, small_object& other)
                {
                    this_.construct_from<T>(::std::move(other.get<T>()));
                }

                static constexpr void copy_construct_impl(small_object& this_, small_object& other)
                {
                    this_.construct_from<T>(other.get<T>());
                }

                static constexpr void move_assign_impl(small_object& this_, small_object& other)
                {
                    if(!other.has_value()) return;
                    this_.emplace(::std::move(other.get<T>()));
                }

                static constexpr void copy_assign_impl(small_object& this_, small_object& other)
                {
                    if(!other.has_value()) return;
                    this_.emplace(other.get<T>());
                }
            };

        public:
            small_object() = default;

            template<typename T>
            [[nodiscard]] constexpr bool is_same_type() const noexcept
            {
                return type_id<T> == traits_->curent_type;
            }

            [[nodiscard]] constexpr auto type() const noexcept { return traits_->curent_type; }

            template<typename... T>
            constexpr small_object(T&&... t):
                alloc_{.allocators = {static_alloc{}, ::std::forward<T>(t)...}}
            {
            }

            constexpr small_object(const small_object& other):
                alloc_{// clang-format off
                    .allocators = {
                        other.get_sbo(),
                        fallback_traits::
                        select_on_container_copy_construction((other.get_fallback_alloc()))
                    }
                }, // clang-format on
                traits_(other.traits_)
            {
                if(other.has_value()) (*(other.traits_->copy_construct))(other, *this);
            }

            constexpr small_object(small_object&& other) //
                noexcept(noexcept((*(other.traits_->move_construct))(other, *this))):
                alloc_{.allocators = {other.get_sbo(), ::std::move(other.get_fallback_alloc())}},
                traits_(other.traits_)
            {
                if(other.has_value()) (*(other.traits_->move_construct))(other, *this);
            }

            constexpr small_object& operator=(const small_object& other)
            {
                if(this != &other) other.traits_->copy_assign(*this, other);
                return *this;
            }

            constexpr small_object& operator=(small_object&& other) //
                noexcept(noexcept(other.traits_->move_assign(*this, other)))
            {
                if(this != &other) other.traits_->move_assign(*this, other);
                return *this;
            }

            constexpr ~small_object() noexcept { reset(); }

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

            static_alloc alloc_;
            soo_storage* allocated_{};
            ::std::uintmax_t size_{};

            const traits_base* traits_{};
        };
    }

    // template<typename FallbackAlloc>
    // class small_object
    // {
    //     using static_alloc = static_allocator<soo_storage, 1>;
    //     using fallback_traits = allocator_traits<FallbackAlloc>;
    //     using alloc_type = composed_allocator<static_allocator<soo_storage, 1>, FallbackAlloc>;

    //     constexpr void allocate(::std::uintmax_t size)
    //     {
    //         size_ = size;
    //         allocated_ = alloc_.allocate(size_);
    //     }

    //     constexpr void deallocate() noexcept
    //     {
    //         alloc_.deallocate(allocated_, size_);
    //         allocated_ = {};
    //         size_ = 0;
    //     }

    //     template<typename T>
    //     constexpr T* get_ptr() const noexcept
    //     {
    //         return to_other_address<T>(allocated_.ptr);
    //     }

    //     template<typename T>
    //     constexpr T& get() const noexcept
    //     {
    //         return *get_ptr<T>();
    //     }

    //     struct traits_base
    //     {
    //         ::std::string_view curent_type{};
    //         void (*destroy)(small_object&) noexcept = nullptr;
    //         void (*move_construct)(small_object&, small_object&) = nullptr;
    //         void (*copy_construct)(small_object&, small_object&) = nullptr;
    //         void (*move_assign)(small_object&, small_object&) = nullptr;
    //         void (*copy_assign)(small_object&, small_object&) = nullptr;
    //     };

    //     constexpr auto& get_sbo() noexcept { return get<0>(alloc_.allocators); }

    //     constexpr auto& get_fallback_alloc() noexcept { return get<1>(alloc_.allocators); }

    //     template<typename T>
    //     [[nodiscard]] constexpr bool is_same_type() const noexcept
    //     {
    //         return type_id<T> == traits_->curent_type;
    //     }

    //     template<typename T>
    //     constexpr void set_traits() noexcept
    //     {
    //         traits_ = &traits_v<T>;
    //     }

    //     template<typename T, typename... Args>
    //         requires move_assignable<T>
    //     constexpr void assign_as(Args&&... args) //
    //         noexcept(nothrow_move_assignable<T>&& nothrow_constructible_from<T, Args...>)
    //     {
    //         get<T>() = construct<T>(::std::forward<Args>(args)...);
    //     }

    //     template<typename T, typename... Args>
    //     constexpr bool try_assign_as(Args&&... args) //
    //         noexcept(noexcept(assign_as<T>(*::std::declval<Args>()...)))
    //         requires requires { assign_as<T>(*::std::declval<Args>()...); }
    //     {
    //         if(!is_same_type<T>()) return false;

    //         assign_as<T>(::std::forward<Args>(args)...);
    //         return true;
    //     }

    //     template<typename T, typename... Args>
    //     constexpr bool try_assign_as(Args&&...) noexcept
    //     {
    //         return false;
    //     }

    //     template<typename T, typename... Args>
    //     constexpr void overwrite(Args&&... args) //
    //         noexcept(sizeof(T) <= static_alloc::size && nothrow_constructible_from<T, Args...>)
    //     {
    //         destroy();

    //         {
    //             constexpr auto n = ceil_reminder(sizeof(T), sizeof(soo_storage));

    //             if(size_ < n) allocate(n);
    //         }

    //         alloc_.construct(get_ptr<T>(), ::std::forward<Args>(args)...);

    //         set_traits<T>();
    //     }

    //     template<typename T, typename... Args>
    //     constexpr void construct_from(Args&&... args) //
    //         noexcept(sizeof(T) <= static_alloc::size && nothrow_constructible_from<T, Args...>)
    //     {
    //         allocate(ceil_reminder(sizeof(T), sizeof(soo_storage)));
    //         alloc_.construct(get_ptr<T>(), ::std::forward<Args>(args)...);
    //         set_traits<T>();
    //     }

    //     template<typename T, typename... Args>
    //     constexpr void emplace_impl(Args&&... args) noexcept( //
    //         noexcept(
    //             try_assign_as<T>(::std::declval<Args>()...),
    //             overwrite<T>(::std::declval<Args>()...)
    //         )
    //     )
    //     {
    //         if(try_assign_as<T>(::std::forward<Args>(args)...)) return;
    //         overwrite<T>(::std::forward<Args>(args)...);
    //     }

    //     constexpr void destroy() noexcept { (*(traits_->destroy))(*this); }

    //     template<typename T>
    //     struct traits : traits_base
    //     {
    //         constexpr traits() noexcept:
    //             traits_base{
    //                 type_id<T>,
    //                 &destroy_impl,
    //                 &move_construct_impl,
    //                 &copy_construct_impl,
    //                 &move_assign_impl,
    //                 &copy_assign_impl //
    //             }
    //         {
    //         }

    //         static constexpr void destroy_impl(small_object& this_)
    //         {
    //             if(!this_.has_value()) return;

    //             this_.alloc_.destroy(this_.allocated_.index, this_.get_ptr<T>());
    //             this_.traits_ = nullptr;
    //         }

    //         static constexpr void move_construct_impl(small_object& this_, small_object& other)
    //         {
    //             this_.construct_from<T>(::std::move(other.get<T>()));
    //         }

    //         static constexpr void copy_construct_impl(small_object& this_, small_object& other)
    //         {
    //             this_.construct_from<T>(other.get<T>());
    //         }

    //         static constexpr void move_assign_impl(small_object& this_, small_object& other)
    //         {
    //             if(!other.has_value()) return;

    //             if constexpr(fallback_traits::propagate_on_container_move_assignment::value) {}
    //         }

    //         static constexpr void copy_assign_impl(small_object& this_, small_object& other)
    //         {
    //             auto& value = other.get<T>();

    //             if constexpr(fallback_traits::propagate_on_container_copy_assignment::value)
    //             {
    //                 if(this_.get_fallback_alloc() != other.get_fallback_alloc() &&
    //                    this_.allocated_.index == 1)
    //                 {
    //                     destroy_impl(this_);
    //                     this_.deallocate();

    //                     this_.get_fallback_alloc() = other.get_fallback_alloc();

    //                     this_.overwrite<T>(value);

    //                     return;
    //                 }

    //                 this_.get_fallback_alloc() = other.get_fallback_alloc();
    //             }

    //             if(this_.try_assign_as<T>(value)) return;

    //             destroy_impl(this_);
    //             this_.overwrite<T>(value);

    //             this_.traits_ = other.traits_;
    //         }
    //     };

    // public:
    //     small_object() = default;

    //     template<typename... T>
    //     constexpr small_object(T&&... t):
    //         alloc_{.allocators = {static_alloc{}, ::std::forward<T>(t)...}}
    //     {
    //     }

    //     constexpr small_object(const small_object& other):
    //         alloc_{// clang-format off
    //                 .allocators = {
    //                     other.get_sbo(),
    //                     fallback_traits::
    //                     select_on_container_copy_construction((other.get_fallback_alloc()))
    //                 }
    //             }, // clang-format on
    //         traits_(other.traits_)
    //     {
    //         if(other.has_value()) (*(other.traits_->copy_construct))(other, *this);
    //     }

    //     constexpr small_object(small_object&& other) //
    //         noexcept(noexcept((*(other.traits_->move_construct))(other, *this))):
    //         alloc_{.allocators = {other.get_sbo(), ::std::move(other.get_fallback_alloc())}},
    //         traits_(other.traits_)
    //     {
    //         if(other.has_value()) (*(other.traits_->move_construct))(other, *this);
    //     }

    //     constexpr small_object& operator=(const small_object& other)
    //     {
    //         if(this != &other) other.traits_->copy_assign(*this, other);
    //         return *this;
    //     }

    //     constexpr small_object& operator=(small_object&& other) //
    //         noexcept(noexcept(other.traits_->move_assign(*this, other)))
    //     {
    //         if(this != &other) other.traits_->move_assign(*this, other);
    //         return *this;
    //     }

    //     constexpr ~small_object() noexcept { reset(); }

    //     template<::std::copy_constructible T, typename... Args>
    //         requires ::std::constructible_from<T, Args...>
    //     constexpr void emplace(Args&&... args) //
    //         noexcept(noexcept(emplace_impl<T>(::std::forward<Args>(args)...)))
    //     {
    //         emplace_impl<T>(::std::forward<Args>(args)...);
    //     }

    //     constexpr void reset() noexcept
    //     {
    //         destroy();
    //         deallocate();
    //     }

    //     [[nodiscard]] constexpr bool has_value() const noexcept { return traits_ != nullptr; }

    // private:
    //     template<typename T>
    //     static constexpr traits<T> traits_v{};

    //     alloc_type alloc_;
    //     typename alloc_type::alloc_ret allocated_{};
    //     ::std::uintmax_t size_{};

    //     const traits_base* traits_;
    // };

    using small_object = details::small_object;
}