#ifndef AUDIOLIMITER_H
#define AUDIOLIMITER_H

#include <atomic>
#include <cstdint>
#include <vector>

// Gain stage followed by a look-ahead brickwall limiter, so audio can be
// boosted past 100% without clipping. Processes interleaved float frames.
class AudioLimiter
{
public:
    void configure(int sampleRate, int channels);
    void reset();

    // Thread safe.
    void setGain(float gain) { m_targetGain.store(gain); }

    void process(float *frames, int count);

    int latencyFrames() const { return m_lookahead; }

private:
    static constexpr float Ceiling = 0.9886f; // -0.1 dBFS

    int m_channels = 0;
    int m_lookahead = 1;
    float m_gainCoef = 1.0f;
    float m_releaseCoef = 1.0f;

    std::atomic<float> m_targetGain{ 1.0f };
    float m_gain = 1.0f;
    float m_released = 1.0f;

    std::vector<float> m_delay;
    int m_delayPos = 0;

    std::vector<int64_t> m_minIndex;
    std::vector<float> m_minValue;
    int m_minHead = 0;
    int m_minSize = 0;
    int64_t m_index = 0;

    std::vector<float> m_box;
    int m_boxPos = 0;
    double m_boxSum = 0.0;
};

#endif // AUDIOLIMITER_H
