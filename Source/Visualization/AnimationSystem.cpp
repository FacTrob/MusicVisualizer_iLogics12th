#include "AnimationSystem.h"
#include "../Utils/MathUtils.h"
#include <algorithm>
#include <cmath>

Animation::Animation()
    : m_duration(1.0f), m_currentTime(0.0f), m_loop(false)
{
}

Animation::~Animation()
{
}

void Animation::AddKeyframe(float time, float value, EasingType easing)
{
    AnimationKeyframe keyframe;
    keyframe.time = time;
    keyframe.value = value;
    keyframe.easing = easing;

    m_keyframes.push_back(keyframe);

    // Sort keyframes by time
    std::sort(m_keyframes.begin(), m_keyframes.end(),
        [](const AnimationKeyframe& a, const AnimationKeyframe& b) {
            return a.time < b.time;
        });
}

float Animation::Evaluate(float time)
{
    if (m_keyframes.empty())
        return 0.0f;

    if (m_keyframes.size() == 1)
        return m_keyframes[0].value;

    // Handle looping
    if (m_loop && time > m_duration)
        time = fmodf(time, m_duration);

    // Clamp time to duration
    time = std::clamp(time, 0.0f, m_duration);

    // Find surrounding keyframes
    for (size_t i = 0; i < m_keyframes.size() - 1; ++i)
    {
        if (time >= m_keyframes[i].time && time <= m_keyframes[i + 1].time)
        {
            float t = (time - m_keyframes[i].time) / (m_keyframes[i + 1].time - m_keyframes[i].time);
            t = ApplyEasing(t, m_keyframes[i].easing);

            return MathUtils::Lerp(m_keyframes[i].value, m_keyframes[i + 1].value, t);
        }
    }

    // Return last keyframe value if beyond range
    return m_keyframes.back().value;
}

bool Animation::IsComplete(float time) const
{
    return !m_loop && time >= m_duration;
}

float Animation::ApplyEasing(float t, EasingType easing)
{
    switch (easing)
    {
    case EasingType::Linear:
        return t;
    case EasingType::EaseIn:
        return MathUtils::EaseIn(t);
    case EasingType::EaseOut:
        return MathUtils::EaseOut(t);
    case EasingType::EaseInOut:
        return MathUtils::EaseInOut(t);
    case EasingType::Bounce:
        return MathUtils::Bounce(t);
    case EasingType::Elastic:
        return sinf(13.0f * MathUtils::HALF_PI * t) * powf(2.0f, 10.0f * (t - 1.0f));
    default:
        return t;
    }
}

AnimationSystem::AnimationSystem()
    : m_bassVelocity(0.0f)
    , m_midVelocity(0.0f)
    , m_trebleVelocity(0.0f)
    , m_currentBass(0.0f)
    , m_currentMid(0.0f)
    , m_currentTreble(0.0f)
    , m_time(0.0f)
{
}

AnimationSystem::~AnimationSystem()
{
}

void AnimationSystem::Update(float deltaTime)
{
    m_time += deltaTime;
}

float AnimationSystem::SmoothDamp(float current, float target, float& velocity, float smoothTime, float deltaTime)
{
    smoothTime = std::max(0.0001f, smoothTime);
    float omega = 2.0f / smoothTime;
    float x = omega * deltaTime;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);

    float change = current - target;
    float originalTo = target;

    float maxChange = 1000.0f * smoothTime;
    change = std::clamp(change, -maxChange, maxChange);
    target = current - change;

    float temp = (velocity + omega * change) * deltaTime;
    velocity = (velocity - omega * temp) * exp;
    float output = target + (change + temp) * exp;

    if (originalTo - current > 0.0f == output > originalTo)
    {
        output = originalTo;
        velocity = (output - originalTo) / deltaTime;
    }

    return output;
}

float AnimationSystem::SpringDamp(float current, float target, float& velocity, float stiffness, float damping, float deltaTime)
{
    float force = stiffness * (target - current);
    velocity += force * deltaTime;
    velocity *= powf(damping, deltaTime);
    return current + velocity * deltaTime;
}

float AnimationSystem::GetBassResponse(float amplitude, float deltaTime)
{
    // Slower, heavier response for bass
    m_currentBass = SmoothDamp(m_currentBass, amplitude, m_bassVelocity, 0.2f, deltaTime);
    return m_currentBass;
}

float AnimationSystem::GetMidResponse(float amplitude, float deltaTime)
{
    // Medium response for mid frequencies
    m_currentMid = SmoothDamp(m_currentMid, amplitude, m_midVelocity, 0.1f, deltaTime);
    return m_currentMid;
}

float AnimationSystem::GetTrebleResponse(float amplitude, float deltaTime)
{
    // Quick, snappy response for treble
    m_currentTreble = SmoothDamp(m_currentTreble, amplitude, m_trebleVelocity, 0.05f, deltaTime);
    return m_currentTreble;
}

void AnimationSystem::Reset()
{
    m_bassVelocity = 0.0f;
    m_midVelocity = 0.0f;
    m_trebleVelocity = 0.0f;
    m_currentBass = 0.0f;
    m_currentMid = 0.0f;
    m_currentTreble = 0.0f;
    m_time = 0.0f;
}