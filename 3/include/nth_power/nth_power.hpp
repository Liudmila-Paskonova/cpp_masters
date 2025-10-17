#pragma once

#include "matrix/matrix.hpp"

namespace nth_power
{
template <matrix::NthPowerType T>
T
nth_power(T x, unsigned n)
{
    T acc = 1;
    if (n == 0) {
        return acc;
    }
    if (n == 1) {
        return x;
    }
    while (n > 0) {
        if ((n & 0x1) == 0x1) {
            acc *= x;
            n -= 1;
        }
        x *= x;
        n /= 2;
    }
    return acc;
}

} // namespace nth_power
