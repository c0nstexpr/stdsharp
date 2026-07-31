#pragma once
#include "forward_like.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename Type>
    class deducing_t
    {
        deducing_t() = default;

        template<typename Self>
            requires std::invocable<forward_like_fn<Self, Type>, Self>
        [[nodiscard]] static constexpr decltype(auto) deducing_cast(Self&& self) noexcept
        {
            return forward_like<Self, Type>(self);
        }

        friend Type;
    };
}

namespace what
{
    using namespace stdsharp;

    class u : deducing_t<u>
    {
        void fun1() { (void)deducing_cast(*this); }
    };

    class v : deducing_t<v>
    {
        void fun2() { (void)deducing_cast(*this); }
    };

    class w : u, v, deducing_t<w>
    {
    public:
        void fun3(this auto&& self) { (void)deducing_cast(self); }
    };

    void foo() { w{}.fun3(); }
}

#include "../compilation_config_out.h"
