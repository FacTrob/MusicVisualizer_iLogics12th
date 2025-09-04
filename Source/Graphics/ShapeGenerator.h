#pragma once
#include "Renderer.h"
#include "ShapeTypes.h"
#include <vector>

class ShapeGenerator
{
public:
    ShapeGenerator();
    ~ShapeGenerator();

    std::vector<Vertex> GenerateShape(ShapeType type, float radius, const XMFLOAT2& center);
    std::vector<Vertex> GenerateCircle(float radius, const XMFLOAT2& center, int segments = 64);
    std::vector<Vertex> GeneratePolygon(int sides, float radius, const XMFLOAT2& center);
    std::vector<Vertex> GenerateStar(int points, float outerRadius, float innerRadius, const XMFLOAT2& center);

    // Advanced shapes
    std::vector<Vertex> GenerateSpiral(float radius, const XMFLOAT2& center, int turns, int segments);
    std::vector<Vertex> GenerateWave(float amplitude, float frequency, const XMFLOAT2& center, int segments);

private:
    float DegreesToRadians(float degrees);
};