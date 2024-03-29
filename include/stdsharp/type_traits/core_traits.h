#pragma once

#include <array>
#include <string_view>
#include <algorithm>

#include <meta/meta.hpp>
#include <nameof.hpp>
#include <range/v3/functional/invoke.hpp>

#include "../utility/adl_proof.h"
#include "../macros.h"

#include "../compilation_config_in.h"

using namespace std::literals;

namespace stdsharp
{
    using ranges::invoke;

    template<typename ReturnT>
    struct invoke_r_fn
    {
        template<typename... Args, typename Func>
            requires std::is_invocable_r_v<ReturnT, Func, Args...>
        [[nodiscard]] constexpr ReturnT operator()(Func&& func, Args&&... args) const
            noexcept(std::is_nothrow_invocable_r_v<ReturnT, Func, Args...>)
        {
            return
#if __cpp_lib_invoke_r >= 202106L
                std::invoke_r<ReturnT>(cpp_forward(func), cpp_forward(args)...)
#else
                static_cast<ReturnT>(invoke(cpp_forward(func), cpp_forward(args)...))
#endif
                    ;
        };
    };

    template<typename ReturnT>
    inline constexpr invoke_r_fn<ReturnT> invoke_r{};

    template<bool Noexcept, typename Ret, typename... Args>
    using func_pointer = Ret (*)(Args...) noexcept(Noexcept);

    template<typename T, typename U>
    using is_derived_from = std::is_base_of<U, T>;

    template<typename T>
    concept nttp_able = requires {
        []<T...> {}();
    };

    template<template<typename...> typename T, typename... Args>
    struct deduction
    {
        using type = T<std::decay_t<Args>...>;
    };

    template<template<typename...> typename T, typename... Args>
    using deduction_t = typename deduction<T, Args...>::type;

    template<template<typename...> typename T>
    struct make_template_type_fn
    {
        template<typename... Args>
            requires requires { typename deduction<T, Args...>::type; }
        using type = deduction_t<T, Args...>;

        template<typename... Args>
            requires std::constructible_from<type<Args...>, Args...>
        constexpr auto operator()(Args&&... args) const
            noexcept(std::is_nothrow_constructible_v<type<Args...>, Args...>)
        {
            return type<Args...>{cpp_forward(args)...};
        }
    };

    enum class ref_qualifier
    {
        none,
        lvalue,
        rvalue
    };
}

namespace stdsharp::details
{
    template<bool, typename, typename, typename...>
    struct mem_func_pointer;

#define MEM_FUNC_POINTER(cv, ref)                                       \
    template<bool Noexcept, typename Ret, typename T, typename... Args> \
    struct mem_func_pointer<Noexcept, Ret, cv T ref, Args...>           \
    {                                                                   \
        using type = Ret (T::*)(Args...) cv ref noexcept(Noexcept);     \
    };

    MEM_FUNC_POINTER(const, &)
    MEM_FUNC_POINTER(const, &&)
    MEM_FUNC_POINTER(volatile, &)
    MEM_FUNC_POINTER(volatile, &&)
    MEM_FUNC_POINTER(const volatile, &)
    MEM_FUNC_POINTER(const volatile, &&)
    MEM_FUNC_POINTER(, &)
    MEM_FUNC_POINTER(, &&)

#undef MEM_FUNC_POINTER
}

namespace stdsharp
{
    template<bool Noexcept, typename Ret, typename T, typename... Args>
    using mem_func_pointer = details::mem_func_pointer<Noexcept, Ret, T, Args...>::type;

    template<typename T>
    inline constexpr auto get_ref_qualifier = std::is_lvalue_reference_v<T> ? //
        ref_qualifier::lvalue :
        std::is_rvalue_reference_v<T> ? ref_qualifier::rvalue : ref_qualifier::none;

    enum class expr_req
    {
        ill_formed,
        well_formed,
        no_exception,
    };

    constexpr auto get_expr_req(const bool well_formed, const bool no_exception = false) noexcept
    {
        return well_formed ? //
            no_exception ? //
                expr_req::no_exception :
                expr_req::well_formed :
            expr_req::ill_formed;
    }

    constexpr bool is_well_formed(const expr_req req) noexcept
    {
        return req >= expr_req::well_formed;
    }

    constexpr bool is_noexcept(const expr_req req) noexcept
    {
        return req >= expr_req::no_exception;
    }

    template<typename T, typename Ret, typename... Args>
    inline constexpr auto invocable_r_test =
        get_expr_req(std::is_invocable_r_v<T, Ret, Args...>, std::is_nothrow_invocable_r_v<T, Ret, Args...>);

    template<typename T, typename... Args>
    inline constexpr auto invocable_test =
        get_expr_req(std::is_invocable_v<T, Args...>, std::is_nothrow_invocable_v<T, Args...>);

    template<typename T, typename... Args>
    inline constexpr auto constructible_from_test =
        get_expr_req(std::is_constructible_v<T, Args...>, std::is_nothrow_constructible_v<T, Args...>);

    template<typename T, bool Const>
    using apply_const = std::conditional_t<Const, std::add_const_t<T>, T>;

    template<typename T, bool Volatile>
    using apply_volatile = std::conditional_t<Volatile, std::add_volatile_t<T>, T>;

    template<typename T, ref_qualifier ref>
    using apply_ref = std::conditional_t<
        std::ranges::equal_to{}(ref, ref_qualifier::lvalue),
        std::add_lvalue_reference_t<T>,
        std::conditional_t<
            std::ranges::equal_to{}(ref, ref_qualifier::rvalue),
            std::add_rvalue_reference_t<T>,
            T // clang-format off
        >
    >; // clang-format on

    template<typename T, bool Const, bool Volatile, ref_qualifier ref>
    using apply_qualifiers = apply_ref<apply_volatile<apply_const<T, Const>, Volatile>, ref>;

    template<typename...>
    [[nodiscard]] constexpr auto dependent_false(const auto&... /*unused*/) noexcept
    {
        return false;
    }

    using ignore_t = decltype(std::ignore);

    inline constexpr struct empty_t : ignore_t
    {
        using ignore_t::operator=;

        empty_t() = default;

        constexpr empty_t(const auto&... /*unused*/) noexcept {}

        constexpr bool operator==(const empty_t /*unused*/) const noexcept { return true; }
    } empty;

    template<typename T>
    using add_const_lvalue_ref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

    template<typename T, typename U>
    using ref_align_t = std::conditional_t<
        std::is_lvalue_reference_v<T>,
        std::add_lvalue_reference_t<U>, // clang-format off
        std::conditional_t<std::is_rvalue_reference_v<T>, std::add_rvalue_reference_t<U>, U>
    >; // clang-format on

    template<typename T, typename U>
    using const_align_t = std::conditional_t<
        std::is_const_v<std::remove_reference_t<T>>,
        std::add_const_t<U>, // clang-format off
        U
    >; // clang-format on

    template<typename T, typename U>
    using volatile_align_t = std::conditional_t<
        std::is_volatile_v<std::remove_reference_t<T>>,
        std::add_volatile_t<U>, // clang-format off
        U
    >; // clang-format on

    template<typename T, typename U>
    using cv_ref_align_t = ref_align_t<T, volatile_align_t<T, const_align_t<T, U>>>;

    template<auto Value>
    using constant = std::integral_constant<decltype(Value), Value>;

    template<bool conditional, decltype(auto) Left, auto>
    inline constexpr auto conditional_v = Left;

    template<decltype(auto) Left, decltype(auto) Right>
    inline constexpr auto conditional_v<false, Left, Right> = Right;

    template<typename T>
    concept constant_value = cpp_is_constexpr(T::value);

    template<std::size_t I>
    using index_constant = std::integral_constant<std::size_t, I>;

    template<typename T>
    struct basic_type_constant : std::type_identity<T>
    {
        template<typename U>
        [[nodiscard]] constexpr bool
            operator==(const basic_type_constant<U> /*unused*/) const noexcept
        {
            return std::same_as<T, U>;
        }
    };

    template<typename T>
    basic_type_constant(std::type_identity<T>) -> basic_type_constant<T>;

    template<typename T>
    using type_constant = adl_proof_t<basic_type_constant, T>;

    template<typename T>
    struct deduction<type_constant, std::type_identity<T>>
    {
        using type = type_constant<T>;
    };

    template<typename T>
    inline constexpr make_template_type_fn<type_constant> make_type_constant{};

    template<auto...>
    struct regular_value_sequence;

    template<typename... T>
    struct basic_type_sequence
    {
        [[nodiscard]] static constexpr auto size() noexcept { return sizeof...(T); }
    };

    template<typename... T>
    using regular_type_sequence = adl_proof_t<basic_type_sequence, T...>;

    template<auto... V>
    struct regular_value_sequence
    {
        [[nodiscard]] static constexpr std::size_t size() noexcept { return sizeof...(V); }
    };
}

namespace stdsharp::details
{
    template<template<auto...> typename T, decltype(auto)... V>
    consteval regular_value_sequence<V...> to_regular_value_sequence(const T<V...>&);

    template<typename T, decltype(auto)... V>
    consteval regular_value_sequence<V...>
        to_regular_value_sequence(std::integer_sequence<T, V...>);

    template<auto From, auto PlusF, std::size_t... I>
        requires requires { regular_value_sequence<invoke(PlusF, From, I)...>{}; }
    consteval regular_value_sequence<invoke(PlusF, From, I)...>
        make_value_sequence(std::index_sequence<I...>) noexcept;

    template<std::array Array, std::size_t... Index>
        requires nttp_able<typename decltype(Array)::value_type>
    consteval regular_value_sequence<(Array[Index])...>
        array_to_sequence(std::index_sequence<Index...>);

    template<
        constant_value T,
        std::ranges::input_range Range = decltype(T::value),
        nttp_able ValueType = std::ranges::range_value_t<Range> // clang-format off
    > // clang-format on
        requires requires {
            requires std::ranges::sized_range<Range>;
            requires std::copyable<ValueType>;
        }
    struct rng_to_sequence
    {
        static constexpr auto rng = T::value;
        static constexpr auto size = std::ranges::size(rng);

        static constexpr auto array = []
        {
            if constexpr( //
                    requires { array_to_sequence<rng>(std::make_index_sequence<size>{}); }
                )
                return rng;
            else
            {
                std::array<ValueType, size> array{};
                std::ranges::copy(rng, array.begin());
                return array;
            }
        }();

        using type = decltype(array_to_sequence<array>(std::make_index_sequence<array.size()>{}));
    };

    template<typename... T, template<typename...> typename Inner, typename... U>
    consteval Inner<T...> template_rebind_impl(Inner<U...>);
}

namespace stdsharp
{
    template<typename Seq>
    using to_regular_value_sequence =
        decltype(details::to_regular_value_sequence(std::declval<Seq>()));

    template<typename T, T Size>
    using make_integer_sequence = to_regular_value_sequence<std::make_integer_sequence<T, Size>>;

    template<std::size_t N>
    using make_index_sequence = make_integer_sequence<std::size_t, N>;

    template<auto From, std::size_t Size, auto PlusF = std::plus{}>
    using make_value_sequence_t = decltype( //
        details::make_value_sequence<From, PlusF>(std::make_index_sequence<Size>{})
    );

    template<typename... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;

    template<typename Rng>
    using rng_to_sequence = ::meta::_t<details::rng_to_sequence<Rng>>;

    template<decltype(auto) Rng>
    using rng_v_to_sequence = rng_to_sequence<constant<Rng>>;

    template<template<typename> typename T, typename... Ts>
    struct ttp_expend : T<Ts>...
    {
        ttp_expend() = default;

        template<typename... Us>
            requires(std::constructible_from<T<Ts>, Us> && ...)
        constexpr ttp_expend(Us&&... us) //
            noexcept((std::is_nothrow_constructible_v<T<Ts>, Us> && ...)):
            T<Ts>(cpp_forward(us))...
        {
        }
    };

    template<template<typename> typename T, typename... Ts>
    ttp_expend(T<Ts>...) -> ttp_expend<T, Ts...>;

    template<template<typename> typename T, typename Seq>
    using make_ttp_expend_by = ::meta::_t< //
        decltype( //
            []<template<typename...> typename Inner, typename... U>(const Inner<U...>&)
            {
                return std::type_identity<ttp_expend<T, U...>>{}; //
            }(std::declval<Seq>())
        ) // clang-format off
    >; // clang-format on

    template<template<auto> typename T, decltype(auto)... V>
    struct nttp_expend : T<V>...
    {
        nttp_expend() = default;

        template<typename... Us>
            requires(std::constructible_from<T<V>, Us> && ...)
        constexpr nttp_expend(Us&&... us) //
            noexcept((std::is_nothrow_constructible_v<T<V>, Us> && ...)):
            T<V>(cpp_forward(us))...
        {
        }
    };

    template<template<auto> typename T, decltype(auto)... V>
    nttp_expend(T<V>...) -> nttp_expend<T, V...>;

    template<template<auto> typename T, std::size_t Size>
    using make_nttp_expend = decltype( //
        []<template<auto...> typename Inner, decltype(auto)... V>(const Inner<V...>) //
        {
            return std::type_identity<nttp_expend<T, V...>>{}; //
        }(make_index_sequence<Size>{})
    );

    template<typename Template, typename... T>
        requires requires { details::template_rebind_impl<T...>(std::declval<Template>()); }
    using template_rebind = decltype(details::template_rebind_impl<T...>(std::declval<Template>()));
}

namespace stdsharp::cpo::inline cpo_impl
{
    void get(auto&&) = delete;

    template<std::size_t I>
    struct get_element_fn
    {
        [[nodiscard]] constexpr decltype(auto) operator()(auto&& t) const
            noexcept(noexcept(cpp_forward(t).template get<I>()))
            requires requires { cpp_forward(t).template get<I>(); }
        {
            return cpp_forward(t).template get<I>();
        }

        [[nodiscard]] constexpr decltype(auto) operator()(auto&& t) const
            noexcept(noexcept(get<I>(cpp_forward(t))))
            requires requires //
        {
            get<I>(cpp_forward(t));
            requires !requires { cpp_forward(t).template get<I>(); };
        }
        {
            return get<I>(cpp_forward(t));
        }
    };

    template<std::size_t I>
    inline constexpr get_element_fn<I> get_element;
}

namespace stdsharp
{
    template<std::size_t I, typename T>
    using get_element_t = decltype(cpo::get_element<I>(std::declval<T>()));

    template<typename T>
    inline constexpr std::string_view type_id = ::nameof::nameof_full_type<T>();

    namespace literals
    {
        template<std::size_t Size>
        struct ltr : std::array<char, Size>
        {
        private:
            using array_t = const char (&)[Size]; // NOLINT(*-avoid-c-arrays)

        public:
            using base = std::array<char, Size>;
            using base::base;

            constexpr ltr(array_t arr) noexcept { std::ranges::copy(arr, base::begin()); }

            constexpr ltr& operator=(array_t arr) noexcept
            {
                std::ranges::copy(arr, base::begin());
                return *this;
            }

            [[nodiscard]] constexpr operator std::string_view() const noexcept
            {
                return {base::data(), Size - 1};
            }

            [[nodiscard]] constexpr auto to_string_view() const noexcept
            {
                return static_cast<std::string_view>(*this); //
            }
        };

        template<std::size_t Size>
        ltr(const char (&)[Size]) -> ltr<Size>; // NOLINT(*-avoid-c-arrays)

        template<ltr Ltr>
        [[nodiscard]] constexpr auto operator""_ltr() noexcept
        {
            return Ltr;
        }
    }
}

namespace stdsharp::details
{
    template<typename T>
    struct ebo_union
    {
        union type
        {
            T v;
        };
    };

    template<typename T>
        requires std::is_empty_v<T>
    struct ebo_union<T>
    {
        union type
        {
            STDSHARP_NO_UNIQUE_ADDRESS T v;
        };
    };

    template<
        typename T,
        typename Tuple,
        typename = std::make_index_sequence<std::tuple_size_v<Tuple>>>
    struct piecewise_traits;

    template<typename T, typename Tuple, std::size_t... I>
    struct piecewise_traits<T, Tuple, std::index_sequence<I...>>
    {
        static constexpr auto constructible_from =
            requires { requires std::constructible_from<T, get_element_t<I, Tuple>...>; };

        static constexpr auto nothrow_constructible_from =
            requires { requires std::is_nothrow_constructible_v<T, get_element_t<I, Tuple>...>; };
    };
}

namespace stdsharp
{
    template<typename T>
    using ebo_union = details::ebo_union<T>::type;

    template<typename T, typename Tuple>
    concept piecewise_constructible_from = //
        details::piecewise_traits<T, Tuple>::constructible_from;

    template<typename T, typename Tuple>
    concept piecewise_nothrow_constructible_from = //
        details::piecewise_traits<T, Tuple>::nothrow_constructible_from;
}

namespace meta::extension
{
    template<typename Fn, template<auto...> typename T, decltype(auto)... V>
        requires requires { typename Fn::template invoke<V...>; }
    struct apply<Fn, T<V...>>
    {
        using type = Fn::template invoke<V...>;
    };
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::basic_type_sequence<T...>>
    {
        static constexpr auto value = ::stdsharp::basic_type_sequence<T...>::size();
    };

    template<::stdsharp::adl_proofed_for<::stdsharp::basic_type_sequence> T>
    struct tuple_size<T> : tuple_size<typename T::basic_type_sequence>
    {
    };

    template<auto... V>
    struct tuple_size<::stdsharp::regular_value_sequence<V...>>
    {
        static constexpr auto value = ::stdsharp::regular_value_sequence<V...>::size();
    };
}

#include "../compilation_config_out.h"