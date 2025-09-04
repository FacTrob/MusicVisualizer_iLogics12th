#pragma once
#include "../Graphics/ShapeTypes.h"
#include <DirectXMath.h>
#include <vector>
#include <memory>

using namespace DirectX;

// Forward declarations
class ShapeGenerator;
struct FrequencyBand;
struct Vertex;

struct PatternShape
{
    ShapeType type = ShapeType::Circle;
    XMFLOAT2 position = { 0.0f, 0.0f };
    float radius = 0.0f;
    float rotation = 0.0f;
    float amplitude = 0.0f;
    XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    std::vector<Vertex> vertices;
    bool active = false;
};

class GeometricPatterns
{
public:
    GeometricPatterns();
    ~GeometricPatterns();

    void Initialize();
    void Update(const std::vector<FrequencyBand>& frequencyBands, float deltaTime);
    void GeneratePatterns(const std::vector<FrequencyBand>& frequencyBands);

    const std::vector<PatternShape>& GetShapes() const { return m_shapes; }

    void SetPatternStyle(int style) { m_patternStyle = style; }

private:
    void UpdateShapeFromFrequency(PatternShape& shape, const FrequencyBand& band, float deltaTime);
    ShapeType GetShapeTypeFromFrequency(float frequency);
    XMFLOAT2 GetPositionFromFrequency(float frequency, int index);

    std::vector<PatternShape> m_shapes;
    std::unique_ptr<ShapeGenerator> m_shapeGenerator;
    int m_patternStyle;
    float m_time;
};