#pragma once

#include <cmath>

namespace Utils
{
    [[nodiscard]] bool equals(float a, float b, float epsilon = 0.0001f)
    {
        return std::abs(a - b) < epsilon;
    }
}