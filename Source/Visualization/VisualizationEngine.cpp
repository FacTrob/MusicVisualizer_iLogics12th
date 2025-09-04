#include "VisualizationEngine.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/ColorManager.h"
#include "GeometricPatterns.h"
#include "AnimationSystem.h"
#include "../Audio/FrequencyAnalyzer.h"
#include "../Utils/MathUtils.h"
#include <algorithm>

// std::max 매크로 충돌 방지
#ifdef max
#undef max
#endif

VisualizationEngine::VisualizationEngine()
    : m_renderer(nullptr)
    , m_visualizationMode(0)
    , m_currentBackgroundColor(0.0f, 0.0f, 0.0f)
    , m_time(0.0f)
    , m_bassLevel(0.0f)
    , m_midLevel(0.0f)
    , m_trebleLevel(0.0f)
{
}

VisualizationEngine::~VisualizationEngine()
{
    Shutdown();
}

bool VisualizationEngine::Initialize(Renderer* renderer)
{
    if (!renderer)
        return false;

    m_renderer = renderer;

    // Initialize subsystems
    m_colorManager = std::make_unique<ColorManager>();
    m_geometricPatterns = std::make_unique<GeometricPatterns>();
    m_animationSystem = std::make_unique<AnimationSystem>();

    // Initialize patterns
    m_geometricPatterns->Initialize();
    m_geometricPatterns->SetPatternStyle(0); // Start with grid layout

    // Set initial color mode
    m_colorManager->SetColorMode(ColorMode::Rainbow);

    return true;
}

void VisualizationEngine::Update(const std::vector<FrequencyBand>& frequencyBands, float deltaTime)
{
    m_time += deltaTime;

    // Update subsystems
    m_colorManager->Update(deltaTime);
    m_animationSystem->Update(deltaTime);

    // Calculate frequency levels
    float newBassLevel = 0.0f;
    float newMidLevel = 0.0f;
    float newTrebleLevel = 0.0f;

    for (const auto& band : frequencyBands)
    {
        if (band.frequency < 250.0f)
            newBassLevel = (newBassLevel > band.smoothedAmplitude) ? newBassLevel : band.smoothedAmplitude;
        else if (band.frequency < 4000.0f)
            newMidLevel = (newMidLevel > band.smoothedAmplitude) ? newMidLevel : band.smoothedAmplitude;
        else
            newTrebleLevel = (newTrebleLevel > band.smoothedAmplitude) ? newTrebleLevel : band.smoothedAmplitude;
    }

    // Apply animation system for smooth transitions
    m_bassLevel = m_animationSystem->GetBassResponse(newBassLevel, deltaTime);
    m_midLevel = m_animationSystem->GetMidResponse(newMidLevel, deltaTime);
    m_trebleLevel = m_animationSystem->GetTrebleResponse(newTrebleLevel, deltaTime);

    // Update background color
    UpdateBackground(frequencyBands);

    // Update geometric patterns
    m_geometricPatterns->Update(frequencyBands, deltaTime);
}

void VisualizationEngine::Render()
{
    if (!m_renderer)
        return;

    // Set background color
    m_renderer->SetBackgroundColor(
        m_currentBackgroundColor.x,
        m_currentBackgroundColor.y,
        m_currentBackgroundColor.z
    );

    // Render shapes
    RenderShapes();
}

void VisualizationEngine::UpdateBackground(const std::vector<FrequencyBand>& frequencyBands)
{
    // Get new background color from color manager
    XMFLOAT3 targetColor = m_colorManager->GetBackgroundColor(m_bassLevel, m_midLevel, m_trebleLevel);

    // Smooth transition to new color
    float colorSpeed = 2.0f;
    m_currentBackgroundColor = MathUtils::LerpColor(m_currentBackgroundColor, targetColor, colorSpeed * 0.016f); // Assuming 60 FPS
}

void VisualizationEngine::RenderShapes()
{
    const auto& shapes = m_geometricPatterns->GetShapes();

    for (const auto& shape : shapes)
    {
        if (!shape.active || shape.vertices.empty() || shape.amplitude < 0.01f)
            continue;

        // Get shape color
        XMFLOAT4 shapeColor = m_colorManager->GetShapeColor(0.0f, shape.amplitude);

        // Render shape as line strip (for outline)
        m_renderer->DrawLineStrip(shape.vertices, shapeColor);
    }
}

void VisualizationEngine::SetColorMode(ColorMode mode)
{
    if (m_colorManager)
    {
        m_colorManager->SetColorMode(mode);
    }
}

void VisualizationEngine::NextVisualizationMode()
{
    m_visualizationMode = (m_visualizationMode + 1) % 3;

    if (m_geometricPatterns)
    {
        m_geometricPatterns->SetPatternStyle(m_visualizationMode);
    }
}

void VisualizationEngine::Shutdown()
{
    m_colorManager.reset();
    m_geometricPatterns.reset();
    m_animationSystem.reset();
    m_renderer = nullptr;
}