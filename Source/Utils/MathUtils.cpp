#include "MathUtils.h"
#include <algorithm>
#include <cmath>

namespace MathUtils
{
    float Lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    float SmoothStep(float edge0, float edge1, float x)
    {
        float t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    float Clamp(float value, float min, float max)
    {
        return std::max(min, std::min(max, value));
    }

    XMFLOAT2 Lerp(const XMFLOAT2& a, const XMFLOAT2& b, float t)
    {
        return XMFLOAT2(
            Lerp(a.x, b.x, t),
            Lerp(a.y, b.y, t)
        );
    }

    XMFLOAT3 Lerp(const XMFLOAT3& a, const XMFLOAT3& b, float t)
    {
        return XMFLOAT3(
            Lerp(a.x, b.x, t),
            Lerp(a.y, b.y, t),
            Lerp(a.z, b.z, t)
        );
    }

    XMFLOAT3 HSVtoRGB(float h, float s, float v)
    {
        float c = v * s;
        float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;

        float r, g, b;

        if (h >= 0.0f && h < 60.0f)
        {
            r = c; g = x; b = 0.0f;
        }
        else if (h >= 60.0f && h < 120.0f)
        {
            r = x; g = c; b = 0.0f;
        }
        else if (h >= 120.0f && h < 180.0f)
        {
            r = 0.0f; g = c; b = x;
        }
        else if (h >= 180.0f && h < 240.0f)
        {
            r = 0.0f; g = x; b = c;
        }
        else if (h >= 240.0f && h < 300.0f)
        {
            r = x; g = 0.0f; b = c;
        }
        else
        {
            r = c; g = 0.0f; b = x;
        }

        return XMFLOAT3(r + m, g + m, b + m);
    }

    XMFLOAT3 LerpColor(const XMFLOAT3& a, const XMFLOAT3& b, float t)
    {
        return XMFLOAT3(
            Lerp(a.x, b.x, t),
            Lerp(a.y, b.y, t),
            Lerp(a.z, b.z, t)
        );
    }

    std::vector<XMFLOAT2> GenerateCircleVertices(float radius, int segments)
    {
        std::vector<XMFLOAT2> vertices;
        vertices.reserve(segments + 1);

        for (int i = 0; i <= segments; ++i)
        {
            float angle = (float)i / segments * TWO_PI;
            vertices.emplace_back(
                radius * cosf(angle),
                radius * sinf(angle)
            );
        }

        return vertices;
    }

    std::vector<XMFLOAT2> GeneratePolygonVertices(float radius, int sides)
    {
        std::vector<XMFLOAT2> vertices;
        vertices.reserve(sides);

        for (int i = 0; i < sides; ++i)
        {
            float angle = (float)i / sides * TWO_PI;
            vertices.emplace_back(
                radius * cosf(angle),
                radius * sinf(angle)
            );
        }

        return vertices;
    }

    std::vector<XMFLOAT2> GenerateStarVertices(float outerRadius, float innerRadius, int points)
    {
        std::vector<XMFLOAT2> vertices;
        vertices.reserve(points * 2);

        for (int i = 0; i < points * 2; ++i)
        {
            float angle = (float)i / (points * 2) * TWO_PI;
            float radius = (i % 2 == 0) ? outerRadius : innerRadius;

            vertices.emplace_back(
                radius * cosf(angle),
                radius * sinf(angle)
            );
        }

        return vertices;
    }

    float EaseInOut(float t)
    {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }

    float EaseIn(float t)
    {
        return t * t;
    }

    float EaseOut(float t)
    {
        return t * (2.0f - t);
    }

    float Bounce(float t)
    {
        if (t < 1.0f / 2.75f)
        {
            return 7.5625f * t * t;
        }
        else if (t < 2.0f / 2.75f)
        {
            float postFix = t -= 1.5f / 2.75f;
            return 7.5625f * postFix * t + 0.75f;
        }
        else if (t < 2.5f / 2.75f)
        {
            float postFix = t -= 2.25f / 2.75f;
            return 7.5625f * postFix * t + 0.9375f;
        }
        else
        {
            float postFix = t -= 2.625f / 2.75f;
            return 7.5625f * postFix * t + 0.984375f;
        }
    }
}