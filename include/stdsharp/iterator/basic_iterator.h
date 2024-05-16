#pragma once

#include "../default_operator.h"

#include <iterator>

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename T>
    concept iterator_has_mem_data = requires(T& v, const T& const_v) {
        v.data();
        const_v.data();
    };
}

namespace stdsharp
{
    template<typename Category = std::random_access_iterator_tag>
        requires std::derived_from<Category, std::input_iterator_tag> ||
                     std::derived_from<Category, std::output_iterator_tag>
    struct STDSHARP_EBO basic_iterator :
        default_operator::arithmetic,
        default_operator::arrow,
        default_operator::subscript,
        default_operator::plus_commutative
    {
    private:
        template<typename T>
        static constexpr auto has_mem_data = requires(T& t) { t.data(); };

    public:
        template<typename T>
        using difference_type = std::iter_difference_t<decltype(std::declval<const T&>().data())>;

        using iterator_category = Category;

// TODO: multidimensional subscript
#if __cpp_multidimensional_subscript >= 202110L
        using subscript::operator[];
#endif

        using increase::operator++;
        using increase::operator--;
        using arithmetic::operator-;

        template<typename T>
            requires has_mem_data<T>
        constexpr auto& operator++(this T& t) noexcept(noexcept(++t.data()))
        {
            ++t.data();
            return t;
        }

        template<typename T>
            requires has_mem_data<T>
        constexpr auto& operator--(this T& t) noexcept(noexcept(--t.data()))
            requires requires { --t.data(); }
        {
            --t.data();
            return t;
        }

        template<typename T>
            requires has_mem_data<T>
        constexpr auto& operator+=(this T& t, const difference_type<T>& diff) noexcept
            requires requires { t.data() += diff; }
        {
            t.data() += diff;
            return t;
        }

        template<typename T>
            requires has_mem_data<T>
        constexpr auto& operator-=(this T& t, const difference_type<T>& diff) noexcept
            requires requires { t.data() -= diff; }
        {
            t.data() -= diff;
            return t;
        }

        template<typename T>
            requires has_mem_data<const T>
        [[nodiscard]] constexpr decltype(auto) operator-(this const T& left, decltype(left) right)
            noexcept(noexcept(left.data() - right.data()))
            requires requires { left.data() - right.data(); }
        {
            return left.data() - right.data();
        }

        template<typename T>
            requires has_mem_data<const T>
        [[nodiscard]] constexpr decltype(auto) operator<=>(this const T& left, decltype(left) right)
            noexcept(noexcept(left.data() <=> right.data()))
            requires requires { left.data() <=> right.data(); }
        {
            return left.data() <=> right.data();
        }

        template<typename T>
            requires has_mem_data<const T>
        [[nodiscard]] constexpr decltype(auto) operator==(this const T& left, decltype(left) right)
            noexcept(noexcept(left.data() == right.data()))
            requires requires { left.data() == right.data(); }
        {
            return left.data() == right.data();
        }

    private:
        static constexpr void not_null(const nullable_pointer auto& ptr) noexcept
        {
            assert_not_null(ptr);
        }

        static constexpr void not_null(const auto& /*unused*/) noexcept {}

    public:
        template<typename T>
            requires has_mem_data<const T>
        [[nodiscard]] constexpr decltype(auto) operator*(this const T& t)
            noexcept(noexcept(*(t.data())))
            requires requires { *(t.data()); }
        {
            const auto& ptr = t.data();
            not_null(ptr);
            return *ptr;
        }

        template<typename T>
            requires has_mem_data<const T>
        [[nodiscard]] constexpr decltype(auto) operator[]( //
            this const T& t,
            const difference_type<T>& diff
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