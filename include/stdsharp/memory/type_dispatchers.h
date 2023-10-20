#pragma once

#include "../utility/dispatcher.h"
#include "allocator_traits.h"
#include "../type_traits/special_member.h"
#include "pointer_traits.h"

namespace stdsharp::details
{
    template<special_mem_req Req, typename Allocator>
    struct type_dispatchers_base
    {
        static constexpr auto req = Req;

        using cp_ctor = dispatcher<req.copy_construct, void, const void*, void*, Allocator&>;
        using mov_ctor = dispatcher<req.move_construct, void, void*, void*, Allocator&>;
        using cp_assign = dispatcher<req.copy_assign, void, const void*, void*>;
        using mov_assign = dispatcher<req.move_assign, void, void*, void*>;
        using dtor = dispatcher<req.destruct, void, void*, Allocator&>;
        using swapper = dispatcher<req.swap, void, void*, void*>;
    };

    template<special_mem_req Req>
    struct type_dispatchers_base<Req, void>
    {
        static constexpr auto req = Req;

        using cp_ctor = dispatcher<req.copy_construct, void, const void*, void*>;
        using mov_ctor = dispatcher<req.move_construct, void, void*, void*>;
        using cp_assign = dispatcher<req.copy_assign, void, const void*, void*>;
        using mov_assign = dispatcher<req.move_assign, void, void*, void*>;
        using dtor = dispatcher<req.destruct, void, void*>;
        using swapper = dispatcher<req.swap, void, void*, void*>;
    };

    template<typename T>
    struct type_dispatchers_traits
    {
        static constexpr auto ptr_cast = pointer_cast<T>;

        template<typename Allocator>
        static constexpr special_mem_req req{
            get_expr_req(
                allocator_traits<Allocator>::template mov_constructible<T>,
                allocator_traits<Allocator>::template nothrow_mov_constructible<T> //
            ),
            get_expr_req(
                allocator_traits<Allocator>::template cp_constructible<T>,
                allocator_traits<Allocator>::template nothrow_cp_constructible<T> //
            ),
            get_expr_req(move_assignable<T>, nothrow_move_assignable<T>),
            get_expr_req(copy_assignable<T>, nothrow_copy_assignable<T>),
            get_expr_req(
                allocator_traits<Allocator>::template destructible<T>,
                allocator_traits<Allocator>::template nothrow_destructible<T> //
            ),
            get_expr_req(std::swappable<T>, nothrow_swappable<T>)
        };

        template<>
        static constexpr auto req<void> = special_mem_req::for_type<T>();

        static constexpr struct
        {
            template<typename Allocator>
                requires(allocator_traits<Allocator>::template cp_constructible<T>)
            constexpr void operator()(const void* src, void* dst, Allocator& alloc) const
                noexcept(allocator_traits<Allocator>::template nothrow_cp_constructible<T>)

            {
                allocator_traits<Allocator>::construct(alloc, dst, *ptr_cast(src));
            }

            constexpr void operator()(const void* src, void* dst) const
                noexcept(nothrow_copy_constructible<T>)
                requires std::copy_constructible<T>
            {
                std::ranges::construct_at(dst, *ptr_cast(src));
            }
        } cp_construct{};

        static constexpr struct
        {
            template<typename Allocator>
                requires(allocator_traits<Allocator>::template mov_constructible<T>)
            constexpr void operator()(void* src, void* dst, Allocator& alloc) const
                noexcept(allocator_traits<Allocator>::template nothrow_mov_constructible<T>)
            {
                allocator_traits<Allocator>::construct(alloc, dst, cpp_move(*ptr_cast(src)));
            }

            constexpr void operator()(void* src, void* dst) const
                noexcept(nothrow_move_constructible<T>)
                requires std::move_constructible<T>
            {
                std::ranges::construct_at(dst, cpp_move(*ptr_cast(src)));
            }
        } mov_construct{};

        static constexpr struct
        {
            constexpr void operator()(const void* src, void* dst) const
                noexcept(nothrow_copy_assignable<T>)
                requires copy_assignable<T>
            {
                *ptr_cast(dst) = *ptr_cast(src);
            }
        } cp_assign{};

        static constexpr struct
        {
            constexpr void operator()(void* src, void* dst) const
                noexcept(nothrow_move_assignable<T>)
                requires move_assignable<T>
            {
                *ptr_cast(dst) = cpp_move(*ptr_cast(src));
            }
        } mov_assign{};

        static constexpr struct
        {
            template<typename Allocator>
                requires(allocator_traits<Allocator>::template destructible<T>)
            constexpr void operator()(void* ptr, Allocator& alloc) const
                noexcept(allocator_traits<Allocator>::template nothrow_destructible<T>)
            {
                allocator_traits<Allocator>::destroy(alloc, ptr_cast(ptr));
            }

            constexpr void operator()(void* ptr) const noexcept(std::destructible<T>)
                requires std::is_destructible_v<T>
            {
                std::ranges::destroy_at(ptr_cast(ptr));
            }
        } dtor{};

        static constexpr struct
        {
            constexpr void operator()(void* lhs, void* rhs) const noexcept(nothrow_swappable<T>)
                requires std::swappable<T>
            {
                std::ranges::swap(*ptr_cast(lhs), *ptr_cast(rhs));
            }
        } swapper{};
    };
}

namespace stdsharp
{
    template<special_mem_req Req, typename Allocator>
        requires(std::same_as<void, Allocator> || allocator_req<Allocator>)
    struct type_dispatchers : details::type_dispatchers_base<Req, Allocator>
    {
        static constexpr auto req = Req;

        static constexpr type_dispatchers empty{};

    private:
        using type_dispatchers_base = details::type_dispatchers_base<req, Allocator>;

    public:
        using typename type_dispatchers_base::cp_ctor;
        using typename type_dispatchers_base::mov_ctor;
        using typename type_dispatchers_base::cp_assign;
        using typename type_dispatchers_base::mov_assign;
        using typename type_dispatchers_base::dtor;
        using typename type_dispatchers_base::swapper;

    private:
        cp_ctor cp_ctor_;
        mov_ctor mov_ctor_;
        cp_assign cp_assign_;
        mov_assign mov_assign_;
        dtor dtor_;
        swapper swapper_;

    public:
        type_dispatchers() = default;

        template<typename T>
            requires(details::type_dispatchers_traits<T>::req >= req)
        explicit constexpr type_dispatchers(const std::type_identity<T> /*unused*/) noexcept:
            cp_ctor_(details::type_dispatchers_traits<T>::cp_construct),
            mov_ctor_(details::type_dispatchers_traits<T>::mov_construct),
            cp_assign_(details::type_dispatchers_traits<T>::cp_assign),
            mov_assign_(details::type_dispatchers_traits<T>::mov_assign),
            dtor_(details::type_dispatchers_traits<T>::dtor),
            swapper_(details::type_dispatchers_traits<T>::swapper)
        {
        }

        constexpr void
            copy_construct(const void* src, void* dst, not_same_as<void> auto& alloc) const
            noexcept(is_noexcept(req.copy_construct))
            requires(is_well_formed(req.copy_construct))
        {
            cp_ctor_(src, dst, alloc);
        }

        constexpr void copy_construct(const void* src, void* dst) const
            noexcept(is_noexcept(req.copy_construct))
            requires(is_well_formed(req.copy_construct))
        {
            cp_ctor_(src, dst);
        }

        constexpr void move_construct(void* src, void* dst, not_same_as<void> auto& alloc) const
            noexcept(is_noexcept(req.move_construct))
            requires(is_well_formed(req.move_construct))
        {
            mov_ctor_(src, dst, alloc);
        }

        constexpr void move_construct(void* src, void* dst) const
            noexcept(is_noexcept(req.move_construct))
            requires(is_well_formed(req.move_construct))
        {
            mov_ctor_(src, dst);
        }

        constexpr void copy_assign(const void* src, void* dst) const
            noexcept(is_noexcept(req.copy_assign))
            requires(is_well_formed(req.copy_assign))
        {
            cp_assign_(src, dst);
        }

        constexpr void move_assign(void* src, void* dst) const
            noexcept(is_noexcept(req.move_assign))
            requires(is_well_formed(req.move_assign))
        {
            mov_assign_(src, dst);
        }

        constexpr void destruct(void* ptr, not_same_as<void> auto& alloc) const
            noexcept(is_noexcept(req.destruct))
            requires(is_well_formed(req.destruct))
        {
            dtor_(ptr, alloc);
        }

        constexpr void destruct(void* ptr) const noexcept(is_noexcept(req.destruct))
            requires(is_well_formed(req.destruct))
        {
            dtor_(ptr);
        }

        constexpr void do_swap(void* lhs, void* rhs) const noexcept(is_noexcept(req.swap))
            requires(is_well_formed(req.swap))
        {
            swapper_(lhs, rhs);
        }

        bool operator==(const type_dispatchers&) const = default;

        constexpr operator bool() const noexcept { return *this == empty; }
    };
}