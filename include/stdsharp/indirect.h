#pragma once
#include <iterator>

#include "concepts/concepts.h"
#include "iterator/iterator.h"

namespace stdsharp
{
    template<::std::indirectly_readable T>
    class const_indirect : public ::std::iterator_traits<T>
    {
    protected:
        T t_;

    private:
        using base = ::std::iterator_traits<T>;

        template<typename U>
        static constexpr auto conversion(const const_indirect<U>& other) noexcept
        {
            return other.t_;
        }

        template<typename U>
        static constexpr decltype(auto) conversion(const_indirect<U>&& other) noexcept
        {
            return ::std::move(other.t_);
        }

    public:
        using const_reference = iterator::iter_const_reference_t<T>;
        using typename base::reference;

        template<typename U>
            requires ::std::constructible_from<T, U>
        constexpr explicit const_indirect(U&& u) //
            noexcept(concepts::nothrow_constructible_from<T, U>):
            t_(::std::forward<U>(u))
        {
        }

        template<typename U>
            requires requires { T{conversion(::std::declval<U>())}; }
        constexpr explicit const_indirect(U&& u) //
            noexcept(noexcept(T{conversion(::std::declval<U>())})):
            t_(conversion(::std::forward<U>(u)))
        {
        }

        template<typename U>
            requires requires { t_ = conversion(::std::declval<U>()); }
        constexpr const_indirect& operator=(U&& u) //
            noexcept(noexcept(t_ = conversion(::std::declval<U>())))
        {
            t_ = conversion(::std::forward<U>(u));
        }

        [[nodiscard]] constexpr const_reference operator*() const noexcept { return *t_; }

        [[nodiscard]] constexpr const_reference get() const noexcept { return *t_; }
    };

    template<typename T>
    const_indirect(T&&) -> const_indirect<T>;

    template<typename T>
    class indirect : public const_indirect<T>
    {
        using base = const_indirect<T>;
        using base::t_;

    public:
        using base::base;
        using typename base::reference;

        [[nodiscard]] constexpr reference operator*() noexcept { return *t_; }

        [[nodiscard]] constexpr reference get() noexcept { return *t_; }
    };

    template<typename T>
    indirect(T&&) -> indirect<T>;
}