#pragma once
#include <DirectXMath.h>
#include <memory>
#include <vector>

using namespace DirectX;

// Forward declarations
class Renderer;
class ColorManager;
class GeometricPatterns;
class AnimationSystem;
struct FrequencyBand;

enum class ColorMode;

class VisualizationEngine
{
public:
    VisualizationEngine();
    ~VisualizationEngine();

    bool Initialize(Renderer* renderer);
    void Update(const std::vector<FrequencyBand>& frequencyBands, float deltaTime);
    void Render();
    void Shutdown();

    // Visualization controls
    void SetVisualizationMode(int mode) { m_visualizationMode = mode; }
    void SetColorMode(ColorMode mode);
    void NextVisualizationMode();

private:
    void UpdateBackground(const std::vector<FrequencyBand>& frequencyBands);
    void RenderShapes();

    Renderer* m_renderer;
    std::unique_ptr<ColorManager> m_colorManager;
    std::unique_ptr<GeometricPatterns> m_geometricPatterns;
    std::unique_ptr<AnimationSystem> m_animationSystem;

    int m_visualizationMode;
    XMFLOAT3 m_currentBackgroundColor;
    float m_time;

    // Cached frequency levels for smooth animation
    float m_bassLevel;
    float m_midLevel;
    float m_trebleLevel;
};