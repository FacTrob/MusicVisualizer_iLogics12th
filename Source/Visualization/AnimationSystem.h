#pragma once
#include <vector>
#include <functional>

enum class EasingType
{
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Bounce,
    Elastic
};

struct AnimationKeyframe
{
    float time;
    float value;
    EasingType easing;
};

class Animation
{
public:
    Animation();
    ~Animation();

    void AddKeyframe(float time, float value, EasingType easing = EasingType::Linear);
    void SetDuration(float duration) { m_duration = duration; }
    void SetLoop(bool loop) { m_loop = loop; }

    float Evaluate(float time);
    bool IsComplete(float time) const;

    void Reset() { m_currentTime = 0.0f; }

private:
    float ApplyEasing(float t, EasingType easing);

    std::vector<AnimationKeyframe> m_keyframes;
    float m_duration;
    float m_currentTime;
    bool m_loop;
};

class AnimationSystem
{
public:
    AnimationSystem();
    ~AnimationSystem();

    void Update(float deltaTime);

    // Smooth value transitions
    float SmoothDamp(float current, float target, float& velocity, float smoothTime, float deltaTime);
    float SpringDamp(float current, float target, float& velocity, float stiffness, float damping, float deltaTime);

    // Animation curves for frequency responses
    float GetBassResponse(float amplitude, float deltaTime);
    float GetMidResponse(float amplitude, float deltaTime);
    float GetTrebleResponse(float amplitude, float deltaTime);

    void Reset();

private:
    float m_bassVelocity;
    float m_midVelocity;
    float m_trebleVelocity;

    float m_currentBass;
    float m_currentMid;
    float m_currentTreble;

    float m_time;
};