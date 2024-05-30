#pragma once
#include <random>

//Math functions
class MathHelper
{
public://模板函数
    template<typename T>
    static T Random(T min, T max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        if constexpr (std::is_integral<T>::value) {
            std::uniform_int_distribution<T> dis(min, max);
            return dis(gen);
        } else {
            std::uniform_real_distribution<T> dis(min, max);
            return dis(gen);
        }
    }


    static double Random()
    {
        return Random<double>(0.f, 1.f);
    }

    template<typename T>
    static T Clamp(T value, T min, T max)
    {
        if (value < min)
            return min;
        if (value > max)
            return max;
        return value;
    }


    template<typename T>
    static T Lerp(T a, T b, float t)
    {
        return (1 - t) * a + t * b;
    }

    template<typename T>
    static T SmoothStep(T a, T b, float t)
    {
        t = Clamp(t, 0.0f, 1.0f);
        t = t * t * (3 - 2 * t);
        return Lerp(a, b, t);
    }


    template<typename T>
    static T SmoothDamp(T current, T target, T& currentVelocity, float smoothTime, float deltaTime)
    {
        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        float change = current - target;
        float originalTo = target;
        float temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        float output = target + (change + temp) * exp;
        if (originalTo - current > 0.0f == output - target > 0.0f)
        {
            if (temp > 0.0f)
                output = originalTo;
            else
                output = current;
            currentVelocity = (output - originalTo) / deltaTime;
        }
        return output;
    }


    template<typename T>
    static T MoveTowards(T current, T target, T maxDelta)
    {
        if (Abs(target - current) <= maxDelta)
            return target;
        return current + Sign(target - current) * maxDelta;
    }

    template<typename T>
    static T Sign(T value)
    {
        if (value > 0)
            return 1;
        if (value < 0)
            return -1;
        return 0;
    }

    template<typename T>
    static T Abs(T value)
    {
        if (value < 0)
            return -value;
        return value;
    }

    template<typename T>
    static T Max(T a, T b)
    {
        if (a > b)
            return a;
        return b;
    }

    template<typename T>
    static T Min(T a, T b)
    {
        if (a < b)
            return a;
        return b;
    }

    template<typename T>
    static T Clamp01(T value)
    {
        return Clamp(value, 0, 1);
    }

};



