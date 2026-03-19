#pragma once
#include <algorithm>

class NoiseGate
{
public:

    void initialize(float threshold, float reduction = 0.0f)
    {
        m_threshold = threshold;
        m_reduction = reduction;
    }

    float process(float input)
    {
        if (input < m_threshold)
        {
            // Hard gate → silence
            return input * m_reduction;
        }

        return input;
    }

    void setThreshold(float threshold)
    {
        m_threshold = threshold;
    }

private:

    float m_threshold = 0.01f;   // default noise floor
    float m_reduction = 0.0f;    // 0 = full mute
};