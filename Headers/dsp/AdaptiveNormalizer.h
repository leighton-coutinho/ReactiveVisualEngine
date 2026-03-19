#pragma once
#include <algorithm>
#include <cmath>

class AdaptiveNormalizer
{
public:

    void initialize(float sampleRate,
        float attackMs = 50.0f,
        float releaseMs = 1000.0f,
        float minPeak = 0.001f)
    {
        m_sampleRate = sampleRate;
        m_minPeak = minPeak;

        setAttack(attackMs);
        setRelease(releaseMs);
    }

    float process(float input)
    {
        // --- 1. Track peak (fast up, slow down) ---
        if (input > m_peak)
        {
            // fast attack (new louder signal)
            m_peak = m_attackCoeff * (m_peak - input) + input;
        }
        else
        {
            // slow decay (adapts over time)
            m_peak = m_releaseCoeff * (m_peak - input) + input;
        }

        // Prevent division by very small values
        float safePeak = std::max(m_peak, m_minPeak);

        // --- 2. Normalize ---
        float normalized = input / safePeak;

        // Clamp to [0,1]
        normalized = std::clamp(normalized, 0.0f, 1.0f);

        return normalized;
    }

    float getPeak() const { return m_peak; }

private:

    void setAttack(float ms)
    {
        float t = ms / 1000.0f;
        m_attackCoeff = std::exp(-1.0f / (t * m_sampleRate));
    }

    void setRelease(float ms)
    {
        float t = ms / 1000.0f;
        m_releaseCoeff = std::exp(-1.0f / (t * m_sampleRate));
    }

private:

    float m_sampleRate = 48000.0f;

    float m_peak = 0.01f;
    float m_minPeak = 0.001f;

    float m_attackCoeff = 0.0f;
    float m_releaseCoeff = 0.0f;
};