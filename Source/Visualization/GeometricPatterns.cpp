#include "GeometricPatterns.h"
#include "../Graphics/ShapeGenerator.h"
#include "../Audio/FrequencyAnalyzer.h"
#include "../Utils/MathUtils.h"
#include <cmath>
#include <algorithm>

// std::max 매크로 충돌 방지
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

GeometricPatterns::GeometricPatterns()
    : m_patternStyle(0), m_time(0.0f)
{
    m_shapeGenerator = std::make_unique<ShapeGenerator>();
}

GeometricPatterns::~GeometricPatterns()
{
}

void GeometricPatterns::Initialize()
{
    m_shapes.clear();
    m_shapes.reserve(32); // Reserve space for shapes
}

void GeometricPatterns::Update(const std::vector<FrequencyBand>& frequencyBands, float deltaTime)
{
    m_time += deltaTime;

    // Update existing shapes or create new ones
    if (m_shapes.size() != frequencyBands.size())
    {
        GeneratePatterns(frequencyBands);
    }

    // Update each shape based on its corresponding frequency band
    size_t minSize = (m_shapes.size() < frequencyBands.size()) ? m_shapes.size() : frequencyBands.size();
    for (size_t i = 0; i < minSize; ++i)
    {
        UpdateShapeFromFrequency(m_shapes[i], frequencyBands[i], deltaTime);
    }
}

void GeometricPatterns::GeneratePatterns(const std::vector<FrequencyBand>& frequencyBands)
{
    m_shapes.clear();
    m_shapes.reserve(frequencyBands.size());

    for (size_t i = 0; i < frequencyBands.size(); ++i)
    {
        const auto& band = frequencyBands[i];

        PatternShape shape;
        shape.type = GetShapeTypeFromFrequency(band.frequency);
        shape.position = GetPositionFromFrequency(band.frequency, static_cast<int>(i));
        shape.radius = 0.1f;
        shape.rotation = 0.0f;
        shape.amplitude = band.smoothedAmplitude;
        shape.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        shape.active = true;

        // Generate initial vertices
        shape.vertices = m_shapeGenerator->GenerateShape(shape.type, shape.radius, shape.position);

        m_shapes.push_back(shape);
    }
}

void GeometricPatterns::UpdateShapeFromFrequency(PatternShape& shape, const FrequencyBand& band, float deltaTime)
{
    // Update amplitude with smoothing
    float targetAmplitude = band.smoothedAmplitude;
    shape.amplitude = MathUtils::Lerp(shape.amplitude, targetAmplitude, deltaTime * 10.0f);

    // Update radius based on amplitude
    float baseRadius = 0.05f + shape.amplitude * 0.3f;

    // Add frequency-based scaling
    float freqScale = 1.0f;
    if (band.frequency < 100.0f)        // Sub-bass
        freqScale = 1.5f;
    else if (band.frequency < 250.0f)   // Bass
        freqScale = 1.3f;
    else if (band.frequency < 2000.0f)  // Mid
        freqScale = 1.0f;
    else                                // High
        freqScale = 0.8f;

    shape.radius = baseRadius * freqScale;

    // Update rotation
    shape.rotation += deltaTime * (1.0f + shape.amplitude * 2.0f);

    // Update color alpha based on amplitude
    shape.color.w = 0.3f + shape.amplitude * 0.7f;

    // Regenerate vertices if needed
    if (shape.active && shape.amplitude > 0.01f)
    {
        // Apply rotation to position
        XMFLOAT2 rotatedPos = shape.position;

        if (m_patternStyle == 1) // Circular arrangement
        {
            float angle = m_time * 0.5f + band.frequency * 0.001f;
            float distance = 0.6f + shape.amplitude * 0.2f;
            rotatedPos.x = cosf(angle) * distance;
            rotatedPos.y = sinf(angle) * distance;
        }

        shape.vertices = m_shapeGenerator->GenerateShape(shape.type, shape.radius, rotatedPos);

        // Apply rotation to vertices
        if (shape.rotation != 0.0f)
        {
            float cosR = cosf(shape.rotation);
            float sinR = sinf(shape.rotation);

            for (auto& vertex : shape.vertices)
            {
                float x = vertex.position.x - rotatedPos.x;
                float y = vertex.position.y - rotatedPos.y;

                vertex.position.x = rotatedPos.x + (x * cosR - y * sinR);
                vertex.position.y = rotatedPos.y + (x * sinR + y * cosR);
            }
        }
    }
}

ShapeType GeometricPatterns::GetShapeTypeFromFrequency(float frequency)
{
    // Map frequency ranges to different shapes
    if (frequency < 60.0f)        // Sub-bass
        return ShapeType::Circle;
    else if (frequency < 250.0f)  // Bass
        return ShapeType::Square;
    else if (frequency < 500.0f)  // Low-mid
        return ShapeType::Triangle;
    else if (frequency < 2000.0f) // Mid
        return ShapeType::Pentagon;
    else if (frequency < 4000.0f) // High-mid
        return ShapeType::Hexagon;
    else if (frequency < 8000.0f) // Presence
        return ShapeType::Octagon;
    else                          // Brilliance
        return ShapeType::Star;
}

XMFLOAT2 GeometricPatterns::GetPositionFromFrequency(float frequency, int index)
{
    switch (m_patternStyle)
    {
    case 0: // Grid layout
    {
        int cols = 8;
        int row = index / cols;
        int col = index % cols;

        float x = -0.8f + (col * 0.2f);
        float y = 0.8f - (row * 0.2f);

        return XMFLOAT2(x, y);
    }

    case 1: // Circular layout
    {
        float angle = (float)index / 16.0f * MathUtils::TWO_PI;
        float radius = 0.7f;

        return XMFLOAT2(
            radius * cosf(angle),
            radius * sinf(angle)
        );
    }

    case 2: // Frequency-based horizontal
    {
        float normalizedFreq = std::clamp(logf(frequency / 20.0f) / logf(20000.0f / 20.0f), 0.0f, 1.0f);
        float x = -0.9f + normalizedFreq * 1.8f;
        float y = 0.0f;

        return XMFLOAT2(x, y);
    }

    default:
        return XMFLOAT2(0.0f, 0.0f);
    }
}