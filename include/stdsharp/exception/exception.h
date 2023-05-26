#pragma once

#include <exception>
#include <concepts>
#include <array>

namespace stdsharp
{
    template<std::size_t I>
    class aggregate_exceptions : public std::exception
    {
        std::array<std::exception_ptr, I> exceptions_;

    public:
        aggregate_exceptions(const std::array<std::exception_ptr, I>& exceptions) noexcept:
            exceptions_(exceptions)
        {
        }

        [[nodiscard]] const char* what() const noexcept override { return "Aggregate exceptions"; }

        [[nodiscard]] const auto& exceptions() const noexcept { return exceptions_; }
    };

    template<std::size_t I>
    aggregate_exceptions(const std::array<std::exception_ptr, I>&) -> aggregate_exceptions<I>;

    namespace details
    {
        template<std::invocable T, std::size_t I>
        constexpr void aggregate_try(auto& exceptions, T&& t)
        {
            try
            {
                std::invoke(cpp_forward(t));
            }
            catch(...)
            {
                exceptions[I] = std::current_exception();
                throw aggregate_exceptions{exceptions};
            }
        }

        template<std::size_t I, std::invocable T, std::invocable... U>
        constexpr void aggregate_try(auto& exceptions, T&& t, U&&... u)
        {
            try
            {
                std::invoke(cpp_forward(t));
            }
            catch(...)
            {
                exceptions[I] = std::current_exception();
                aggregate_try<I + 1, U...>(exceptions, cpp_forward(u)...);
            }
        }
    }

    template<std::invocable... T>
    constexpr void aggregate_try(T&&... t)
    {
        std::array<std::exception_ptr, sizeof...(T)> exceptions;
        details::aggregate_try<0, T...>(exceptions, cpp_forward(t)...);
    }
}