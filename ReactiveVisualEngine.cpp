// ReactiveVisualEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

#include "Headers/audio/WasapiCapture.h"
#include "Headers/dsp/AudioBuffer.h"
#include "Headers/dsp/RMS.h"
#include "Headers/dsp/BandSplitter.h"
#include "Headers/dsp/EnvelopeFollower.h"
#include "Headers/dsp/NoiseGate.h"
#include "Headers/dsp/AdaptiveNormalizer.h"

int main()
{
    WasapiCapture cap;

    if (!cap.initializeDefaultLoopback())
    {
        std::cout << "Failed to initialize audio capture\n";
        return 1;
    }

    std::vector<float> mono;

    BandSplitter splitter;

    EnvelopeFollower bassEnv;
    EnvelopeFollower midEnv;
    EnvelopeFollower highEnv;

    NoiseGate bassGate;
    NoiseGate midGate;
    NoiseGate highGate;

    AdaptiveNormalizer bassNorm;
    AdaptiveNormalizer midNorm;
    AdaptiveNormalizer highNorm;

    bool initialized = false;

    cap.setCallback(
        [&](const float* data, size_t frames, int channels, int sampleRate)
        {
            // 1️⃣ Downmix stereo → mono
            AudioBuffer::downmixToMono(data, frames, channels, mono);

            // 2️⃣ Initialize DSP once (we need sample rate)
            if (!initialized)
            {
                float sr = static_cast<float>(sampleRate);
                splitter.initialize(sr);

                bassEnv.initialize(sr, 10.0f, 200.0f);
                midEnv.initialize(sr, 20.0f, 150.0f);
                highEnv.initialize(sr, 5.0f, 80.0f);

                bassGate.initialize(sr, 0.005f);
                midGate.initialize(sr, 0.003f);
                highGate.initialize(sr, 0.002f);

                bassNorm.initialize(sr);
                midNorm.initialize(sr);
                highNorm.initialize(sr);

                initialized = true;
            }

            float bassValue = 0.0f;
            float midValue = 0.0f;
            float highValue = 0.0f;

            // 3️⃣ Process audio block
            for (float s : mono)
            {
                splitter.process(s);

                float b = bassEnv.process(splitter.bass);
                float m = midEnv.process(splitter.mid);
                float h = highEnv.process(splitter.high);

                // Gate first
                b = bassGate.process(b);
                m = midGate.process(m);
                h = highGate.process(h);

                // THEN normalize
                bassValue = bassNorm.process(b);
                midValue = midNorm.process(m);
                highValue = highNorm.process(h);
            }

            // 4️⃣ Print smoothed output
            std::cout
                << "Bass: " << bassValue
                << "  Mid: " << midValue
                << "  High: " << highValue
                << "\r";
        }
    );

    cap.start();

    std::cout << "Listening to system audio...\n";
    std::cout << "Press ENTER to quit\n";

    std::cin.get();

    cap.stop();

    return 0;
}