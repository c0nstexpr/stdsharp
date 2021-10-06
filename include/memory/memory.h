//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once
#include <memory>

namespace stdsharp::memory
{
    template<typename T>
    concept allocator_req = ::std::copyable<T> && requires
    {
        ::std::allocator_traits<T>{};
    };
}
