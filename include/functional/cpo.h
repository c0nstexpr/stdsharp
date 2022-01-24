//
// Created by BlurringShadow on 2021-9-22.
//
#pragma once
#include "functional/invocable_obj.h"

namespace stdsharp::functional
{
    template<typename Tag, typename... Args>
    concept tag_invocable = requires
    {
        tag_invoke(::std::declval<Tag>(), ::std::declval<Args>()...);
    };

    template<typename Tag, typename... Args>
    concept nothrow_tag_invocable = tag_invocable<Tag, Args...> &&
        noexcept(tag_invoke(::std::declval<Tag>(), ::std::declval<Args>()...));

    template<typename Tag>
    struct cpo_fn : Tag
    {
    private:
        static constexpr struct
        {
            template<typename TagType, typename... Args, ::std::invocable<TagType, Args...> T>
            constexpr decltype(auto) operator()(TagType&& tag, T&& t, Args&&... args) const
                noexcept(concepts::nothrow_invocable<T, TagType, Args...>)
            {
                return ::std::invoke(
                    ::std::forward<T>(t),
                    ::std::forward<TagType>(tag),
                    ::std::forward<Args>(args)... //
                );
            }

            template<typename TagType, typename... Args>
                requires tag_invocable<TagType, Args...>
            constexpr decltype(auto) operator()(TagType&& tag, Args&&... args) const
                noexcept(nothrow_tag_invocable<TagType, Args...>)
            {
                return tag_invoke(::std::forward<TagType>(tag), ::std::forward<Args>(args)...);
            }
        } customized_invoke_{};

        template<typename TagT>
        static constexpr auto get_fn(TagT&& tag) noexcept
        {
            return ::ranges::overload(customized_invoke_, ::std::forward<TagT>(tag));
        }

        template<typename TagT>
        using fn_t = decltype(get_fn(::std::declval<TagT>()));

    public:
        using Tag::Tag;

#define BS_CPO_FN_OPERATOR(const_, ref_)                                                         \
    template<typename... T>                                                                      \
        requires requires { get_fn(::std::declval<const_ Tag ref_>())(::std::declval<T>()...); } \
    constexpr decltype(auto) operator()(T&&... t)                                                \
        const_ ref_ noexcept(concepts::nothrow_invocable<fn_t<const_ Tag ref_>, T...>)           \
    {                                                                                            \
        return get_fn(static_cast<const_ Tag ref_>(*this))(::std::forward<T>(t)...);             \
    }                                                                                            \
                                                                                                 \
    constexpr operator const_ Tag ref_() const_ ref_ noexcept { return *this; }

        BS_CPO_FN_OPERATOR(const, &)
        BS_CPO_FN_OPERATOR(const, &&)
        BS_CPO_FN_OPERATOR(, &&)
        BS_CPO_FN_OPERATOR(, &)

#undef BS_CPO_BASE_FN_OPERATOR
    };
}