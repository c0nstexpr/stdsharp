#pragma once
#include <fstream>
#include <filesystem>

#include "containers/actions.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T>
        struct get_from_stream_fn
        {
            template<typename... Args>
                requires ::std::constructible_from<T, Args...>
            [[nodiscard]] constexpr auto operator()(::std::istream& is, Args&&... args) const
            {
                T t{::std::forward<Args>(args)...};
                is >> t;
                return t;
            }
        };
    }

    template<typename T>
    inline constexpr details::get_from_stream_fn<T> get_from_stream{};

    namespace details
    {
        template<typename T>
            requires ::std::invocable<
                details::get_from_stream_fn<T>,
                ::std::ifstream // clang-format off
            > // clang-format on
        struct read_all_to_container_fn
        {
            template<typename Container = ::std::vector<T>>
                requires ::std::invocable<read_all_to_container_fn, Container&>
            [[nodiscard]] auto& operator()(
                Container& container,
                const ::std::filesystem::path& path //
            ) const
            {
                ::std::ifstream fs{path};
                return (*this)(container, fs);
            }

            template<typename Container = ::std::vector<T>>
                requires ::std::invocable<
                    decltype(actions::emplace_back),
                    Container&,
                    T // clang-format off
                > // clang-format on
            [[nodiscard]] constexpr auto& operator()(Container& container, ::std::istream& is) const
            {
                while(is) actions::emplace_back(container, get_from_stream<T>(is));

                return container;
            }
        };
    }

    template<typename T>
    inline constexpr details::read_all_to_container_fn<T> read_all_to_container{};

    namespace details
    {
        template<typename T, ::std::constructible_from Container>
            requires ::std::invocable<
                details::read_all_to_container_fn<T>,
                ::std::add_lvalue_reference_t<Container>,
                ::std::filesystem::path // clang-format off
            > // clang-format on
        struct read_all_fn
        {
            [[nodiscard]] constexpr auto operator()(::std::istream& is) const
            {
                Container container{};
                return read_all_to_container<T>(container, is);
            }

            [[nodiscard]] auto operator()(const ::std::filesystem::path& path) const
            {
                ::std::ifstream fs{path};
                return (*this)(fs);
            }
        };

        struct read_all_text_fn
        {
            [[nodiscard]] auto operator()(::std::istream& is) const
            {
                using traits_t = ::std::istream::traits_type;

                ::std::string str;
                ::std::getline(is, str, traits_t::to_char_type(traits_t::eof()));

                return str;
            }

            [[nodiscard]] auto operator()(const ::std::filesystem::path& path) const
            {
                ::std::ifstream fs{path};
                return (*this)(fs);
            }
        };
    }

    template<typename T, typename Container = ::std::vector<T>>
    inline constexpr details::read_all_fn<T, Container> read_all{};

    inline constexpr details::read_all_text_fn read_all_text{};
}
