#pragma once

#include "mutex.h"

namespace stdsharp
{
    template<typename T>
    concept shared_mutex = mutex<T> && requires(T t)
    {
    };
}