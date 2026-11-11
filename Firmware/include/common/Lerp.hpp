#pragma once

namespace Lerp
{
    template<typename T>
    T Linear(T a, T b, float t)
    {
        return a + (b - a) * t;
    }

    template<typename T>
    T Cubic(T a, T b, float t)
    {
        return a + (b - a) * (3.f * t * t - 2.f * t * t * t);
    }
    
    template<typename T>
    T EaseIn(T a, T b, float t)
    {
        return a + (b - a) * (t * t);
    }

    template<typename T>
    T EaseOut(T a, T b, float t)
    {
        return a + (b - a) * (1.f - (1.f - t) * (1.f - t));
    }

    template<typename T>
    T EaseInOut(T a, T b, float t)
    {
        return a + (b - a) * (t < 0.5f ? 2.f * t * t : 1.f - pow(-2.f * t + 2.f, 2.f) / 2.f);
    }

    struct BezierHandles
    {
        float c1_x;
        float c1_y;
        float c2_x;
        float c2_y;
    };

    template<typename T>
    T Bezier(T a, T b, float t, BezierHandles handles)
    {
        float c1_x = handles.c1_x;
        float c1_y = handles.c1_y;
        float c2_x = handles.c2_x;
        float c2_y = handles.c2_y;

        float u = 1.f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        float x = uuu * a + 3.f * uu * t * c1_x + 3.f * u * tt * c2_x + ttt * b;
        float y = uuu * a + 3.f * uu * t * c1_y + 3.f * u * tt * c2_y + ttt * b;

        return y;
    }
}