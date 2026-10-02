#include "audiodecoder.h"

AudioDecoder::AudioDecoder(QObject *parent)
    : Decoder(parent)
{
}

AudioDecoder::~AudioDecoder()
{
    releaseResampler();
    releaseFilter();
    av_channel_layout_uninit(&m_dstLayout);
}

void AudioDecoder::setTempo(double tempo)
{
    m_tempo.store(qBound(0.5, tempo, 100.0));
}

void AudioDecoder::setOutput(AudioBufferDevice *device, int sampleRate, int channels, AVSampleFormat format)
{
    m_device = device;
    m_dstRate = sampleRate;
    m_dstChannels = channels;
    m_dstFormat = format;

    av_channel_layout_uninit(&m_dstLayout);
    av_channel_layout_default(&m_dstLayout, channels);

    // Force the resampler to be rebuilt for the new output format.
    releaseResampler();
}

void AudioDecoder::open(QSharedPointer<VideoState> videoState)
{
    m_videoState = videoState;
    m_lastPts = NAN;
    m_outPts = NAN;

    if (!videoState->audio_st || !m_device || !openCodec(videoState->audio_st, 1)) {
        emit finished();
        return;
    }

    m_timeBase = videoState->audio_st->time_base;

    run(videoState->audioq);
}

void AudioDecoder::processFrame(AVFrame *frame)
{
    const double pts = framePts(frame);
    const double frameDuration = double(frame->nb_samples) / qMax(1, frame->sample_rate);

    // Everything between the keyframe the seek landed on and the requested
    // position must not be heard, and must not move the clock.
    if (pts + frameDuration <= m_videoState->skipUntil.load())
        return;

    if (!ensureFilter(frame))
        return;

    if (std::isnan(m_outPts))
        m_outPts = pts;

    if (av_buffersrc_add_frame_flags(m_bufferSrc, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
        return;

    while (av_buffersink_get_frame(m_bufferSink, m_filtered) >= 0) {
        emitPcm(m_filtered);
        av_frame_unref(m_filtered);
    }
}

void AudioDecoder::emitPcm(AVFrame *frame)
{
    if (!ensureResampler(frame))
        return;

    const int64_t delay = swr_get_delay(m_swr, m_srcRate);
    const int maxSamples = static_cast<int>(
        av_rescale_rnd(delay + frame->nb_samples, m_dstRate, m_srcRate, AV_ROUND_UP));

    const int bytesPerSample = av_get_bytes_per_sample(m_dstFormat);
    QByteArray pcm(maxSamples * m_dstChannels * bytesPerSample, Qt::Uninitialized);

    uint8_t *out[1] = { reinterpret_cast<uint8_t *>(pcm.data()) };
    const int converted = swr_convert(
        m_swr, out, maxSamples,
        const_cast<const uint8_t **>(frame->extended_data), frame->nb_samples);

    if (converted <= 0)
        return;

    pcm.resize(converted * m_dstChannels * bytesPerSample);

    // Tempo compresses playback time, so the stream time these samples cover is
    // their playback duration scaled back up by it.
    const double span = double(converted) / m_dstRate * m_graphTempo;
    m_device->append(pcm, m_outPts, span);
    m_outPts += span;
}

bool AudioDecoder::ensureFilter(const AVFrame *frame)
{
    const double wanted = m_tempo.load();

    if (m_graph
        && m_graphRate == frame->sample_rate
        && m_graphFormat == static_cast<AVSampleFormat>(frame->format)
        && av_channel_layout_compare(&m_graphLayout, &frame->ch_layout) == 0) {

        if (!qFuzzyCompare(m_graphTempo, wanted)) {
            char value[32];
            snprintf(value, sizeof(value), "%f", wanted);
            if (avfilter_graph_send_command(m_graph, "tempo", "tempo", value,
                                            nullptr, 0, 0) >= 0) {
                m_graphTempo = wanted;
            }
        }
        return true;
    }

    releaseFilter();

    m_graph = avfilter_graph_alloc();
    m_filtered = av_frame_alloc();
    if (!m_graph || !m_filtered) {
        releaseFilter();
        return false;
    }

    char layout[128];
    av_channel_layout_describe(&frame->ch_layout, layout, sizeof(layout));

    char sourceArgs[256];
    snprintf(sourceArgs, sizeof(sourceArgs),
             "time_base=1/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
             frame->sample_rate, frame->sample_rate,
             av_get_sample_fmt_name(static_cast<AVSampleFormat>(frame->format)), layout);

    char tempoArgs[64];
    snprintf(tempoArgs, sizeof(tempoArgs), "tempo=%f", wanted);

    AVFilterContext *format = nullptr;
    AVFilterContext *tempo = nullptr;

    int ret = avfilter_graph_create_filter(&m_bufferSrc, avfilter_get_by_name("abuffer"),
                                           "in", sourceArgs, nullptr, m_graph);
    if (ret >= 0)
        ret = avfilter_graph_create_filter(&format, avfilter_get_by_name("aformat"),
                                           "format", "sample_fmts=flt", nullptr, m_graph);
    if (ret >= 0)
        ret = avfilter_graph_create_filter(&tempo, avfilter_get_by_name("atempo"),
                                           "tempo", tempoArgs, nullptr, m_graph);
    if (ret >= 0)
        ret = avfilter_graph_create_filter(&m_bufferSink, avfilter_get_by_name("abuffersink"),
                                           "out", nullptr, nullptr, m_graph);

    if (ret >= 0) ret = avfilter_link(m_bufferSrc, 0, format, 0);
    if (ret >= 0) ret = avfilter_link(format, 0, tempo, 0);
    if (ret >= 0) ret = avfilter_link(tempo, 0, m_bufferSink, 0);
    if (ret >= 0) ret = avfilter_graph_config(m_graph, nullptr);

    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "Audio filter graph failed:");
        releaseFilter();
        return false;
    }

    m_graphRate = frame->sample_rate;
    m_graphFormat = static_cast<AVSampleFormat>(frame->format);
    av_channel_layout_copy(&m_graphLayout, &frame->ch_layout);
    m_graphTempo = wanted;

    return true;
}

void AudioDecoder::releaseFilter()
{
    if (m_graph)
        avfilter_graph_free(&m_graph);
    if (m_filtered)
        av_frame_free(&m_filtered);

    m_bufferSrc = nullptr;
    m_bufferSink = nullptr;
    av_channel_layout_uninit(&m_graphLayout);
    m_graphRate = 0;
    m_graphFormat = AV_SAMPLE_FMT_NONE;
}

bool AudioDecoder::ensureResampler(const AVFrame *frame)
{
    if (m_swr
        && m_srcRate == frame->sample_rate
        && m_srcFormat == static_cast<AVSampleFormat>(frame->format)
        && av_channel_layout_compare(&m_srcLayout, &frame->ch_layout) == 0) {
        return true;
    }

    releaseResampler();

    if (av_channel_layout_copy(&m_srcLayout, &frame->ch_layout) < 0)
        return false;

    m_srcRate = frame->sample_rate;
    m_srcFormat = static_cast<AVSampleFormat>(frame->format);

    int ret = swr_alloc_set_opts2(
        &m_swr,
        &m_dstLayout, m_dstFormat, m_dstRate,
        &m_srcLayout, m_srcFormat, m_srcRate,
        0, nullptr);

    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "swr_alloc_set_opts2 failed:");
        return false;
    }

    ret = swr_init(m_swr);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "swr_init failed:");
        releaseResampler();
        return false;
    }

    qDebug() << "Resampling" << m_srcRate << "Hz" << m_srcLayout.nb_channels << "ch"
             << av_get_sample_fmt_name(m_srcFormat) << "->" << m_dstRate << "Hz"
             << m_dstChannels << "ch" << av_get_sample_fmt_name(m_dstFormat);

    return true;
}

double AudioDecoder::framePts(const AVFrame *frame)
{
    int64_t ts = frame->best_effort_timestamp;
    if (ts == AV_NOPTS_VALUE)
        ts = frame->pts;

    double pts;
    if (ts == AV_NOPTS_VALUE) {
        pts = std::isnan(m_lastPts) ? 0.0 : m_lastPts;
    } else {
        pts = ts * av_q2d(m_timeBase);
    }

    m_lastPts = pts + double(frame->nb_samples) / qMax(1, frame->sample_rate);
    return pts;
}

void AudioDecoder::onFlush()
{
    m_lastPts = NAN;
    m_outPts = NAN;
    // Drop the PCM queued for the position we just left.
    if (m_device)
        m_device->flush();
    releaseResampler();
    // The graph still holds audio from before the seek.
    releaseFilter();
}

void AudioDecoder::releaseResampler()
{
    if (m_swr)
        swr_free(&m_swr);
    av_channel_layout_uninit(&m_srcLayout);
    m_srcRate = 0;
    m_srcFormat = AV_SAMPLE_FMT_NONE;
}
