#pragma once
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

namespace MathUtils
{
    // Interpolation functions
    float Lerp(float a, float b, float t);
    float SmoothStep(float edge0, float edge1, float x);
    float Clamp(float value, float min, float max);

    // Vector operations
    XMFLOAT2 Lerp(const XMFLOAT2& a, const XMFLOAT2& b, float t);
    XMFLOAT3 Lerp(const XMFLOAT3& a, const XMFLOAT3& b, float t);

    // Color operations
    XMFLOAT3 HSVtoRGB(float h, float s, float v);
    XMFLOAT3 LerpColor(const XMFLOAT3& a, const XMFLOAT3& b, float t);

    // Geometric functions
    std::vector<XMFLOAT2> GenerateCircleVertices(float radius, int segments);
    std::vector<XMFLOAT2> GeneratePolygonVertices(float radius, int sides);
    std::vector<XMFLOAT2> GenerateStarVertices(float outerRadius, float innerRadius, int points);

    // Animation easing functions
    float EaseInOut(float t);
    float EaseIn(float t);
    float EaseOut(float t);
    float Bounce(float t);

    // Utility constants
    const float PI = 3.14159265359f;
    const float TWO_PI = 6.28318530718f;
    const float HALF_PI = 1.57079632679f;
}