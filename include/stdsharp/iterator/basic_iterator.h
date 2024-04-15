#pragma once

#include "../default_operator.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    struct STDSHARP_EBO basic_iterator :
        default_operator::arithmetic,
        default_operator::arrow,
        default_operator::subscript,
        default_operator::plus_commutative
    {
        template<typename T>
        using difference_type = std::iter_difference_t<decltype(std::declval<const T&>().data())>;

        using subscript::operator[];
        using increase::operator++;
        using increase::operator--;
        using arithmetic::operator+;

        constexpr auto& operator++(this auto& t) noexcept(noexcept(++t.data()))
            requires requires { ++t.data(); }
        {
            ++t.data();
            return t;
        }

        constexpr auto& operator--(this auto& t) noexcept(noexcept(--t.data()))
            requires requires { --t.data(); }
        {
            --t.data();
            return t;
        }

        [[nodiscard]] constexpr auto operator<=>(this const auto& left, const auto& right)
            noexcept(noexcept(left.data() <=> right.data()))
            requires requires { left.data() <=> right.data(); }
        {
            return left.data() <=> right.data();
        }

        [[nodiscard]] constexpr auto operator==(this const auto& left, const auto& right)
            noexcept(noexcept(left.data() == right.data()))
            requires requires { left.data() == right.data(); }
        {
            return left.data() == right.data();
        }

        constexpr auto& operator+=(this auto& t, const difference_type<decltype(t)>& diff) noexcept
            requires requires { t.data() += diff; }
        {
            t.data() += diff;
            return t;
        }

        constexpr auto& operator-=(this auto& t, const difference_type<decltype(t)>& diff) noexcept
            requires requires { t.data() -= diff; }
        {
            t.data() -= diff;
            return t;
        }

        [[nodiscard]] constexpr decltype(auto) operator-( //
            this const auto& left,
            decltype(left) right
        ) noexcept(noexcept(left.data() - right.data()))
            requires requires { left.data() - right.data(); }
        {
            return left.data() - right.data();
        }

    private:
        static constexpr void not_null(const nullable_pointer auto& ptr) noexcept
        {
            assert_not_null(ptr);
        }

        static constexpr void not_null(const auto& /*unused*/) noexcept {}

    public:
        [[nodiscard]] constexpr decltype(auto) operator*(this const auto& t)
            noexcept(noexcept(*(t.data())))
            requires requires { *(t.data()); }
        {
            const auto& ptr = t.data();
            not_null(ptr);
            return *ptr;
        }

        [[nodiscard]] constexpr decltype(auto) operator[]( //
            this const auto& t,
            const difference_type<decltype(t)>& diff
        ) noexcept(noexcept(*(t.data() + diff)))
            requires requires { *(t.data() + diff); }
        {
            const auto& ptr = t.data();
            not_null(ptr);
            return *(ptr + diff);
        }
    };
}

#include "../compilation_config_out.h"