#ifndef DEFS_H
#define DEFS_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QSharedPointer>
#include <QString>
#include <QWaitCondition>

#include <atomic>
#include <cmath>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}

// Frames travel across threads through queued connections, so the pointer type
// has to be known to the meta object system.
Q_DECLARE_METATYPE(AVFrame *)

namespace Sync {
// Above this difference the streams are considered unrelated and the frame is
// shown without waiting.
constexpr double NoSyncThreshold = 10.0;
// Video frames this far behind the master clock are dropped instead of shown.
constexpr double DropThreshold = 0.08;
// Frames within this distance of their deadline are shown right away.
constexpr double DisplayTolerance = 0.002;
// Never skip more than this many frames in a row, so something stays on screen.
constexpr int MaxConsecutiveDrops = 12;
}

// Playback clock expressed in stream time (seconds). It is anchored with set()
// and extrapolated from a monotonic timer, so it keeps running smoothly between
// updates instead of stepping once per frame.
class Clock
{
public:
    Clock() { m_timer.start(); }

    void set(double pts)
    {
        QMutexLocker lock(&m_mutex);
        m_pts = pts;
        m_anchor = monotonic();
        m_valid = !std::isnan(pts);
    }

    // Returns NaN while the clock has no reference yet.
    double get() const
    {
        QMutexLocker lock(&m_mutex);
        if (!m_valid)
            return NAN;
        if (m_paused)
            return m_pts;
        return m_pts + (monotonic() - m_anchor) * m_rate;
    }

    void setPaused(bool paused)
    {
        QMutexLocker lock(&m_mutex);
        if (m_paused == paused)
            return;
        if (paused && m_valid)
            m_pts += (monotonic() - m_anchor) * m_rate;
        m_anchor = monotonic();
        m_paused = paused;
    }

    // Stream seconds per real second: playback speed.
    void setRate(double rate)
    {
        QMutexLocker lock(&m_mutex);
        if (qFuzzyCompare(m_rate, rate))
            return;
        if (m_valid && !m_paused)
            m_pts += (monotonic() - m_anchor) * m_rate;
        m_anchor = monotonic();
        m_rate = rate;
    }

    bool isValid() const
    {
        QMutexLocker lock(&m_mutex);
        return m_valid;
    }

    // Keeps the paused flag: a seek must not resume a paused playback.
    void invalidate()
    {
        QMutexLocker lock(&m_mutex);
        m_valid = false;
        m_pts = NAN;
    }

private:
    double monotonic() const { return m_timer.nsecsElapsed() / 1000000000.0; }

    mutable QMutex m_mutex;
    QElapsedTimer m_timer;
    double m_pts = NAN;
    double m_anchor = 0.0;
    double m_rate = 1.0;
    bool m_paused = false;
    bool m_valid = false;
};

// Bounded packet queue. The demuxer blocks while it is full, a decoder blocks
// while it is empty; both wake up on flush/abort/eof. Every packet carries the
// serial of the playback sequence it belongs to, which is how decoders notice
// that a seek happened and drop what they had buffered.
class PacketQueue
{
public:
    ~PacketQueue() { clearLocked(); }

    void init(int maxPackets, int maxBytes)
    {
        QMutexLocker lock(&m_mutex);
        clearLocked();
        m_maxPackets = maxPackets;
        m_maxBytes = maxBytes;
        m_abort = false;
        m_eof = false;
        m_serial = 0;
        m_cond.wakeAll();
    }

    // Takes ownership of pkt, also when it fails.
    bool put(AVPacket *pkt)
    {
        QMutexLocker lock(&m_mutex);
        while (!m_abort && (m_queue.size() >= m_maxPackets || m_bytes >= m_maxBytes))
            m_cond.wait(&m_mutex);

        if (m_abort) {
            av_packet_free(&pkt);
            return false;
        }

        m_bytes += pkt->size;
        m_queue.enqueue({ pkt, m_serial });
        m_cond.wakeAll();
        return true;
    }

    // Returns false when aborted, or when the queue ran dry after end of file.
    bool get(AVPacket **pkt, int *serial)
    {
        QMutexLocker lock(&m_mutex);
        while (!m_abort && m_queue.isEmpty() && !m_eof)
            m_cond.wait(&m_mutex);

        if (m_abort || m_queue.isEmpty())
            return false;

        const Entry entry = m_queue.dequeue();
        m_bytes -= entry.pkt->size;
        *pkt = entry.pkt;
        *serial = entry.serial;
        m_cond.wakeAll();
        return true;
    }

    void flush()
    {
        QMutexLocker lock(&m_mutex);
        clearLocked();
        ++m_serial;
        m_eof = false;
        m_cond.wakeAll();
    }

    void setEof()
    {
        QMutexLocker lock(&m_mutex);
        m_eof = true;
        m_cond.wakeAll();
    }

    void abort()
    {
        QMutexLocker lock(&m_mutex);
        m_abort = true;
        clearLocked();
        m_cond.wakeAll();
    }

    int serial() const
    {
        QMutexLocker lock(&m_mutex);
        return m_serial;
    }

    bool isAborted() const
    {
        QMutexLocker lock(&m_mutex);
        return m_abort;
    }

private:
    struct Entry {
        AVPacket *pkt;
        int serial;
    };

    void clearLocked()
    {
        while (!m_queue.isEmpty()) {
            Entry entry = m_queue.dequeue();
            av_packet_free(&entry.pkt);
        }
        m_bytes = 0;
    }

    mutable QMutex m_mutex;
    QWaitCondition m_cond;
    QQueue<Entry> m_queue;
    qint64 m_bytes = 0;
    int m_maxPackets = 256;
    qint64 m_maxBytes = 8 * 1024 * 1024;
    int m_serial = 0;
    bool m_abort = false;
    bool m_eof = false;
};

// State shared between the demuxer, the two decoders and the player.
struct VideoState {
    QString fileName;
    // Cover art as the container stores it (JPEG or PNG). Only filled in for
    // files without a video track: it is all there is to show while they play.
    QByteArray coverArt;

    PacketQueue audioq;
    PacketQueue videoq;

    int audio_st_index = -1;
    int video_st_index = -1;

    AVStream *audio_st = nullptr;
    AVStream *video_st = nullptr;

    double duration = 0.0; // seconds
    int frames_count = 0;

    // Audio is the master clock whenever the file has a usable audio track; the
    // external clock covers video only files and the gap right after a seek.
    Clock audioClock;
    Clock externalClock;

    std::atomic_bool hasAudio{ false };
    std::atomic_bool hasVideo{ false };
    std::atomic_bool paused{ false };

    std::atomic_bool seekRequested{ false };
    std::atomic<double> seekTarget{ 0.0 };
    // A seek lands on the keyframe before the target, so both decoders throw
    // away everything they decode until this timestamp.
    std::atomic<double> skipUntil{ 0.0 };
    // Set on seek so the video decoder shows the target frame even while paused.
    std::atomic_bool refreshFrame{ false };
    // The video decoder has a frame ready; audio waits for it so both streams
    // start together.
    std::atomic_bool videoPrimed{ false };
    std::atomic<double> speed{ 1.0 };

    double masterClock() const
    {
        if (hasAudio.load()) {
            const double audio = audioClock.get();
            if (!std::isnan(audio))
                return audio;
        }
        return externalClock.get();
    }

    void setSpeed(double value)
    {
        speed.store(value);
        audioClock.setRate(value);
        externalClock.setRate(value);
    }

    void setPaused(bool value)
    {
        paused.store(value);
        audioClock.setPaused(value);
        externalClock.setPaused(value);
    }

    // Called once the pipeline is fully stopped, before opening another file.
    void reset()
    {
        fileName.clear();
        coverArt.clear();

        audio_st_index = -1;
        video_st_index = -1;
        audio_st = nullptr;
        video_st = nullptr;

        duration = 0.0;
        frames_count = 0;

        hasAudio.store(false);
        hasVideo.store(false);
        seekRequested.store(false);
        seekTarget.store(0.0);
        skipUntil.store(0.0);
        refreshFrame.store(false);

        audioClock.invalidate();
        externalClock.invalidate();
        setPaused(false);
    }
};

#endif // DEFS_H
