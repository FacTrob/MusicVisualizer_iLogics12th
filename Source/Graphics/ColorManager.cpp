#include "ColorManager.h"
#include "../Utils/MathUtils.h"
#include <algorithm>
#include <cmath>

ColorManager::ColorManager()
    : m_colorMode(ColorMode::Rainbow)
    , m_baseHue(240.0f)    // Blue
    , m_saturation(0.8f)
    , m_brightness(0.6f)
    , m_time(0.0f)
{
    // Initialize color palettes
    m_bassColors = {
        XMFLOAT3(0.8f, 0.2f, 0.2f),  // Deep red
        XMFLOAT3(0.8f, 0.4f, 0.0f),  // Orange
        XMFLOAT3(0.6f, 0.0f, 0.8f)   // Purple
    };

    m_midColors = {
        XMFLOAT3(0.2f, 0.8f, 0.2f),  // Green
        XMFLOAT3(0.8f, 0.8f, 0.2f),  // Yellow
        XMFLOAT3(0.0f, 0.6f, 0.8f)   // Cyan
    };

    m_trebleColors = {
        XMFLOAT3(0.2f, 0.2f, 0.8f),  // Blue
        XMFLOAT3(0.8f, 0.2f, 0.8f),  // Magenta
        XMFLOAT3(0.8f, 0.8f, 0.8f)   // White
    };
}

ColorManager::~ColorManager()
{
}

void ColorManager::Update(float deltaTime)
{
    m_time += deltaTime;
}

XMFLOAT3 ColorManager::GetBackgroundColor(float bassLevel, float midLevel, float trebleLevel)
{
    switch (m_colorMode)
    {
    case ColorMode::Static:
        return HSVtoRGB(m_baseHue, m_saturation * 0.3f, m_brightness * 0.2f);

    case ColorMode::Frequency:
    {
        // Blend colors based on frequency levels
        XMFLOAT3 bassColor = XMFLOAT3(0.4f * bassLevel, 0.1f * bassLevel, 0.2f * bassLevel);
        XMFLOAT3 midColor = XMFLOAT3(0.2f * midLevel, 0.4f * midLevel, 0.1f * midLevel);
        XMFLOAT3 trebleColor = XMFLOAT3(0.1f * trebleLevel, 0.2f * trebleLevel, 0.4f * trebleLevel);

        return XMFLOAT3(
            bassColor.x + midColor.x + trebleColor.x,
            bassColor.y + midColor.y + trebleColor.y,
            bassColor.z + midColor.z + trebleColor.z
        );
    }

    case ColorMode::Rainbow:
    {
        XMFLOAT3 rainbowColor = GetRainbowColor(m_time * 0.5f);
        float intensity = (bassLevel + midLevel + trebleLevel) / 3.0f;
        return XMFLOAT3(
            rainbowColor.x * intensity * 0.3f,
            rainbowColor.y * intensity * 0.3f,
            rainbowColor.z * intensity * 0.3f
        );
    }

    case ColorMode::Pulse:
    {
        float pulse = sinf(m_time * 3.0f) * 0.5f + 0.5f;
        float totalLevel = (bassLevel + midLevel + trebleLevel) / 3.0f;
        float intensity = pulse * totalLevel;

        return HSVtoRGB(m_baseHue, m_saturation, intensity * 0.4f);
    }

    default:
        return XMFLOAT3(0.0f, 0.0f, 0.0f);
    }
}

XMFLOAT4 ColorManager::GetShapeColor(float frequency, float amplitude)
{
    XMFLOAT3 color;

    switch (m_colorMode)
    {
    case ColorMode::Static:
        color = HSVtoRGB(m_baseHue, m_saturation, m_brightness);
        break;

    case ColorMode::Frequency:
        color = FrequencyToColor(frequency);
        break;

    case ColorMode::Rainbow:
        color = GetRainbowColor(m_time + frequency * 0.001f);
        break;

    case ColorMode::Pulse:
    {
        float pulse = sinf(m_time * 5.0f + frequency * 0.01f) * 0.5f + 0.5f;
        color = HSVtoRGB(m_baseHue + frequency * 0.1f, m_saturation, pulse * m_brightness);
    }
    break;

    default:
        color = XMFLOAT3(1.0f, 1.0f, 1.0f);
        break;
    }

    // Always return white for shape lines as specified
    return XMFLOAT4(1.0f, 1.0f, 1.0f, amplitude);
}

XMFLOAT3 ColorManager::HSVtoRGB(float h, float s, float v)
{
    // Normalize hue to [0, 360]
    while (h < 0.0f) h += 360.0f;
    while (h >= 360.0f) h -= 360.0f;

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

XMFLOAT3 ColorManager::FrequencyToColor(float frequency)
{
    // Map frequency ranges to colors
    if (frequency < 250.0f)
    {
        // Bass frequencies - red/orange
        float t = frequency / 250.0f;
        return MathUtils::LerpColor(XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 0.5f, 0.0f), t);
    }
    else if (frequency < 2000.0f)
    {
        // Mid frequencies - yellow/green
        float t = (frequency - 250.0f) / (2000.0f - 250.0f);
        return MathUtils::LerpColor(XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), t);
    }
    else
    {
        // High frequencies - cyan/blue/violet
        float t = std::min(1.0f, (frequency - 2000.0f) / 6000.0f);
        return MathUtils::LerpColor(XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT3(0.5f, 0.0f, 1.0f), t);
    }
}

XMFLOAT3 ColorManager::GetRainbowColor(float time)
{
    float hue = fmodf(time * 60.0f, 360.0f); // Complete cycle every 6 seconds
    return HSVtoRGB(hue, 1.0f, 1.0f);
}