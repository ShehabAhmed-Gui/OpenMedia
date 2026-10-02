#include "audiolimiter.h"

#include <algorithm>
#include <cmath>
#include <numeric>

static constexpr double LookaheadSeconds = 0.005;
static constexpr double ReleaseSeconds = 0.050;
static constexpr double GainSmoothSeconds = 0.020;

void AudioLimiter::configure(int sampleRate, int channels)
{
    const int rate = std::max(1, sampleRate);

    m_channels = std::max(0, channels);
    m_lookahead = std::max(1, int(std::lround(rate * LookaheadSeconds)));
    m_gainCoef = float(1.0 - std::exp(-1.0 / (rate * GainSmoothSeconds)));
    m_releaseCoef = float(1.0 - std::exp(-1.0 / (rate * ReleaseSeconds)));

    m_delay.assign(size_t(m_lookahead) * m_channels, 0.0f);
    m_minIndex.assign(m_lookahead + 1, 0);
    m_minValue.assign(m_lookahead + 1, 1.0f);
    m_box.assign(m_lookahead + 1, 1.0f);

    reset();
}

void AudioLimiter::reset()
{
    std::fill(m_delay.begin(), m_delay.end(), 0.0f);
    std::fill(m_box.begin(), m_box.end(), 1.0f);
    m_boxSum = double(m_box.size());
    m_delayPos = 0;
    m_boxPos = 0;
    m_minHead = 0;
    m_minSize = 0;
    m_index = 0;
    m_released = 1.0f;
}

void AudioLimiter::process(float *frames, int count)
{
    if (m_channels == 0 || m_box.empty())
        return;

    const float target = m_targetGain.load();
    const int window = m_lookahead + 1;

    for (int i = 0; i < count; ++i) {
        float *frame = frames + i * m_channels;

        // Snapping makes 100% exactly 1.0, so unboosted audio passes through untouched.
        m_gain += (target - m_gain) * m_gainCoef;
        if (std::abs(target - m_gain) < 1e-6f)
            m_gain = target;

        float peak = 0.0f;
        for (int c = 0; c < m_channels; ++c) {
            frame[c] *= m_gain;
            peak = std::max(peak, std::abs(frame[c]));
        }

        // Full scale at 100%, tightening to the ceiling as the boost rises, so
        // crossing 100% doesn't suddenly limit.
        const float ceiling = std::max(Ceiling, 1.0f / m_gain);
        const float required = peak > ceiling ? ceiling / peak : 1.0f;

        // Monotonic queue: lowest required gain within the look-ahead window.
        while (m_minSize > 0 && m_minIndex[m_minHead] <= m_index - window) {
            m_minHead = (m_minHead + 1) % window;
            --m_minSize;
        }
        while (m_minSize > 0 && m_minValue[(m_minHead + m_minSize - 1) % window] >= required)
            --m_minSize;
        const int back = (m_minHead + m_minSize) % window;
        m_minIndex[back] = m_index;
        m_minValue[back] = required;
        ++m_minSize;
        const float held = m_minValue[m_minHead];

        if (held < m_released) {
            m_released = held;
        } else {
            m_released += (held - m_released) * m_releaseCoef;
            if (held - m_released < 1e-6f)
                m_released = held;
        }

        // Averaging over the same window as the delay ramps the gain down
        // ahead of a peak and guarantees it has reached the required level
        // by the time that peak leaves the delay line.
        m_boxSum += m_released - m_box[m_boxPos];
        m_box[m_boxPos] = m_released;
        if (++m_boxPos == window) {
            m_boxPos = 0;
            m_boxSum = std::accumulate(m_box.begin(), m_box.end(), 0.0);
        }
        const float gain = float(m_boxSum / window);

        float *delayed = m_delay.data() + m_delayPos * m_channels;
        for (int c = 0; c < m_channels; ++c) {
            const float in = frame[c];
            frame[c] = delayed[c] * gain;
            delayed[c] = in;
        }
        m_delayPos = (m_delayPos + 1) % m_lookahead;
        ++m_index;
    }
}
