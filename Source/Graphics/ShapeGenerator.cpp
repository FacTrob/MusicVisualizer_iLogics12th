#include "ShapeGenerator.h"
#include "../Utils/MathUtils.h"
#include <cmath>

ShapeGenerator::ShapeGenerator()
{
}

ShapeGenerator::~ShapeGenerator()
{
}

std::vector<Vertex> ShapeGenerator::GenerateShape(ShapeType type, float radius, const XMFLOAT2& center)
{
    switch (type)
    {
    case ShapeType::Circle:
        return GenerateCircle(radius, center);
    case ShapeType::Triangle:
        return GeneratePolygon(3, radius, center);
    case ShapeType::Square:
        return GeneratePolygon(4, radius, center);
    case ShapeType::Pentagon:
        return GeneratePolygon(5, radius, center);
    case ShapeType::Hexagon:
        return GeneratePolygon(6, radius, center);
    case ShapeType::Octagon:
        return GeneratePolygon(8, radius, center);
    case ShapeType::Star:
        return GenerateStar(5, radius, radius * 0.5f, center);
    default:
        return GenerateCircle(radius, center);
    }
}

std::vector<Vertex> ShapeGenerator::GenerateCircle(float radius, const XMFLOAT2& center, int segments)
{
    std::vector<Vertex> vertices;
    vertices.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i)
    {
        float angle = (float)i / segments * MathUtils::TWO_PI;
        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);

        Vertex vertex;
        vertex.position = XMFLOAT3(x, y, 0.0f);
        vertex.texCoord = XMFLOAT2(0.5f + 0.5f * cosf(angle), 0.5f + 0.5f * sinf(angle));
        vertices.push_back(vertex);
    }

    return vertices;
}

std::vector<Vertex> ShapeGenerator::GeneratePolygon(int sides, float radius, const XMFLOAT2& center)
{
    std::vector<Vertex> vertices;
    vertices.reserve(sides + 1);

    for (int i = 0; i <= sides; ++i)
    {
        float angle = (float)i / sides * MathUtils::TWO_PI;
        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);

        Vertex vertex;
        vertex.position = XMFLOAT3(x, y, 0.0f);
        vertex.texCoord = XMFLOAT2(0.5f + 0.5f * cosf(angle), 0.5f + 0.5f * sinf(angle));
        vertices.push_back(vertex);
    }

    // Close the shape by connecting back to first vertex
    if (!vertices.empty())
    {
        vertices.push_back(vertices[0]);
    }

    return vertices;
}

std::vector<Vertex> ShapeGenerator::GenerateStar(int points, float outerRadius, float innerRadius, const XMFLOAT2& center)
{
    std::vector<Vertex> vertices;
    vertices.reserve(points * 2 + 1);

    for (int i = 0; i < points * 2; ++i)
    {
        float angle = (float)i / (points * 2) * MathUtils::TWO_PI;
        float radius = (i % 2 == 0) ? outerRadius : innerRadius;

        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);

        Vertex vertex;
        vertex.position = XMFLOAT3(x, y, 0.0f);
        vertex.texCoord = XMFLOAT2(0.5f + 0.5f * cosf(angle), 0.5f + 0.5f * sinf(angle));
        vertices.push_back(vertex);
    }

    // Close the shape
    if (!vertices.empty())
    {
        vertices.push_back(vertices[0]);
    }

    return vertices;
}

std::vector<Vertex> ShapeGenerator::GenerateSpiral(float radius, const XMFLOAT2& center, int turns, int segments)
{
    std::vector<Vertex> vertices;
    vertices.reserve(segments);

    for (int i = 0; i < segments; ++i)
    {
        float t = (float)i / (segments - 1);
        float angle = t * turns * MathUtils::TWO_PI;
        float currentRadius = radius * t;

        float x = center.x + currentRadius * cosf(angle);
        float y = center.y + currentRadius * sinf(angle);

        Vertex vertex;
        vertex.position = XMFLOAT3(x, y, 0.0f);
        vertex.texCoord = XMFLOAT2(t, 0.5f);
        vertices.push_back(vertex);
    }

    return vertices;
}

std::vector<Vertex> ShapeGenerator::GenerateWave(float amplitude, float frequency, const XMFLOAT2& center, int segments)
{
    std::vector<Vertex> vertices;
    vertices.reserve(segments);

    float width = 2.0f; // Wave width
    float startX = center.x - width * 0.5f;

    for (int i = 0; i < segments; ++i)
    {
        float t = (float)i / (segments - 1);
        float x = startX + t * width;
        float y = center.y + amplitude * sinf(frequency * t * MathUtils::TWO_PI);

        Vertex vertex;
        vertex.position = XMFLOAT3(x, y, 0.0f);
        vertex.texCoord = XMFLOAT2(t, 0.5f + 0.5f * sinf(frequency * t * MathUtils::TWO_PI));
        vertices.push_back(vertex);
    }

    return vertices;
}

float ShapeGenerator::DegreesToRadians(float degrees)
{
    return degrees * MathUtils::PI / 180.0f;
}