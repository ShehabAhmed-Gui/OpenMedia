#include "frameconverter.h"

FrameConverter::FrameConverter() {}

QByteArray FrameConverter::rawtoPcm(AVFrame *audio)
{
    if (!audio)
        return QByteArray();

    static SwrContext* swr = nullptr;
    static AVChannelLayout outLayout;
    static int outSampleRate = 0;
    static AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;

    // Reinitialize resampler if format changed
    if (!swr ||
        outSampleRate != audio->sample_rate ||
        !av_channel_layout_compare(&outLayout, &audio->ch_layout))
    {
        if (swr)
            swr_free(&swr);

        av_channel_layout_uninit(&outLayout);
        av_channel_layout_default(&outLayout, audio->ch_layout.nb_channels);

        SwrContext* tmp = nullptr;
        if (swr_alloc_set_opts2(
                &tmp,
                &outLayout,                      // output layout
                outSampleFmt,                    // output format
                audio->sample_rate,              // output rate
                &audio->ch_layout,               // input layout
                (AVSampleFormat)audio->format,   // input format
                audio->sample_rate,              // input rate
                0,
                nullptr) < 0)
        {
            return QByteArray();
        }

        if (swr_init(tmp) < 0) {
            swr_free(&tmp);
            return QByteArray();
        }

        swr = tmp;
        outSampleRate = audio->sample_rate;
    }

    int outSamples = av_rescale_rnd(
        swr_get_delay(swr, audio->sample_rate) + audio->nb_samples,
        audio->sample_rate,
        audio->sample_rate,
        AV_ROUND_UP
        );

    int outChannels = outLayout.nb_channels;

    int outBufferSize = av_samples_get_buffer_size(
        nullptr,
        outChannels,
        outSamples,
        outSampleFmt,
        1
        );

    if (outBufferSize < 0)
        return QByteArray();

    QByteArray pcm;
    pcm.resize(outBufferSize);

    uint8_t* outData[1] = {
        reinterpret_cast<uint8_t*>(pcm.data())
    };

    int samplesConverted = swr_convert(
        swr,
        outData,
        outSamples,
        (const uint8_t**)audio->data,
        audio->nb_samples
        );

    if (samplesConverted < 0)
        return QByteArray();

    pcm.resize(samplesConverted * outChannels * av_get_bytes_per_sample(outSampleFmt));
    return pcm;
}

QImage FrameConverter::rawtoQImage(AVFrame *frame)
{
    if (!frame)
        return QImage();

    static SwsContext* sws = nullptr;

    sws = sws_getCachedContext(
        sws,
        frame->width,
        frame->height,
        (AVPixelFormat)frame->format,

        frame->width,
        frame->height,
        AV_PIX_FMT_RGBA,

        SWS_BILINEAR,
        nullptr, nullptr, nullptr
        );

    if (!sws)
        return QImage();

    QImage image(
        frame->width,
        frame->height,
        QImage::Format_RGBA8888
        );

    uint8_t* dstData[4] = { image.bits(), nullptr, nullptr, nullptr };
    int dstLinesize[4] = { static_cast<int>(image.bytesPerLine()), 0, 0, 0 };

    sws_scale(
        sws,
        frame->data,
        frame->linesize,
        0,
        frame->height,
        dstData,
        dstLinesize
        );

    return image;
}
