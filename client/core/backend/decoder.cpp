#include "decoder.h"

Decoder::Decoder(QObject *parent)
    : QObject(parent)
{
}

Decoder::~Decoder()
{
    closeCodec();
}

bool Decoder::openCodec(AVStream *stream, int threadCount)
{
    closeCodec();

    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        qCritical() << "No decoder for codec id" << stream->codecpar->codec_id;
        return false;
    }

    m_ctx = avcodec_alloc_context3(codec);
    if (!m_ctx)
        return false;

    int ret = avcodec_parameters_to_context(m_ctx, stream->codecpar);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "avcodec_parameters_to_context failed:");
        closeCodec();
        return false;
    }

    // Needed for correct frame timestamps.
    m_ctx->pkt_timebase = stream->time_base;
    m_ctx->thread_count = threadCount;

    ret = avcodec_open2(m_ctx, codec, nullptr);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "avcodec_open2 failed:");
        closeCodec();
        return false;
    }

    m_frame = av_frame_alloc();
    if (!m_frame) {
        closeCodec();
        return false;
    }

    qDebug() << "Opened decoder:" << codec->name;
    return true;
}

void Decoder::closeCodec()
{
    if (m_ctx)
        avcodec_free_context(&m_ctx);
    if (m_frame)
        av_frame_free(&m_frame);
}

void Decoder::run(PacketQueue &packets)
{
    if (m_stopRequested) {
        closeCodec();
        emit finished();
        return;
    }

    m_running = true;
    m_queue = &packets;
    m_serial = packets.serial();

    while (m_running) {
        AVPacket *pkt = nullptr;
        int packetSerial = 0;

        if (!packets.get(&pkt, &packetSerial))
            break;

        if (packetSerial != m_serial) {
            m_serial = packetSerial;
            avcodec_flush_buffers(m_ctx);
            onFlush();
        }

        decodePacket(pkt);
        av_packet_free(&pkt);
    }

    // Flush the codec so the tail of the file is played too.
    if (m_running && !packets.isAborted())
        decodePacket(nullptr);

    m_running = false;
    m_queue = nullptr;
    closeCodec();
    emit finished();
}

void Decoder::decodePacket(AVPacket *pkt)
{
    int ret = avcodec_send_packet(m_ctx, pkt);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
        PlaybackLogger::printStringError(ret, "avcodec_send_packet failed:");
        return;
    }

    while (m_running) {
        ret = avcodec_receive_frame(m_ctx, m_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;

        if (ret < 0) {
            PlaybackLogger::printStringError(ret, "avcodec_receive_frame failed:");
            break;
        }

        processFrame(m_frame);
        av_frame_unref(m_frame);
    }
}

void Decoder::prepare()
{
    m_stopRequested = false;
}

void Decoder::stop()
{
    m_stopRequested = true;
    m_running = false;
    if (PacketQueue *packets = m_queue.load())
        packets->abort();
}
