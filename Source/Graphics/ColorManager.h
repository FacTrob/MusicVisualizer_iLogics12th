#pragma once
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

enum class ColorMode
{
    Static,
    Frequency,
    Rainbow,
    Pulse,
    Custom
};

class ColorManager
{
public:
    ColorManager();
    ~ColorManager();

    void Update(float deltaTime);

    XMFLOAT3 GetBackgroundColor(float bassLevel, float midLevel, float trebleLevel);
    XMFLOAT4 GetShapeColor(float frequency, float amplitude);

    void SetColorMode(ColorMode mode) { m_colorMode = mode; }
    void SetBaseHue(float hue) { m_baseHue = hue; }
    void SetSaturation(float saturation) { m_saturation = saturation; }
    void SetBrightness(float brightness) { m_brightness = brightness; }

private:
    XMFLOAT3 HSVtoRGB(float h, float s, float v);
    XMFLOAT3 FrequencyToColor(float frequency);
    XMFLOAT3 GetRainbowColor(float time);

    ColorMode m_colorMode;
    float m_baseHue;
    float m_saturation;
    float m_brightness;
    float m_time;

    // Color palettes
    std::vector<XMFLOAT3> m_bassColors;
    std::vector<XMFLOAT3> m_midColors;
    std::vector<XMFLOAT3> m_trebleColors;
};