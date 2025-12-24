#include "demuxer.h"
#include "playbacklogger.h"

Demuxer::Demuxer()
{
}

Demuxer::ErrorType Demuxer::readPacket(VideoState *state)
{
    AVPacket* pkt = av_packet_alloc();
    if (!fc) {
        qCritical() << "Demuxer isn't opened";
        return DemuxerErrorType::ReadPacketError;
    }

    // Read a packet
    int ok = av_read_frame(fc, pkt);

    if (!pkt) {
        qCritical() << "Packet isn't allocated";
        return DemuxerErrorType::ReadPacketError;
    }

    if (pkt->stream_index == state->video_st_index) {
        state->videoq.append(pkt);
    }
    if (pkt->stream_index == state->audio_st_index) {
        state->audioq.append(pkt);
    }

    if (ok == AVERROR_EOF) {
        qDebug() << "Demuxer finished.";
        return DemuxerErrorType::MediaEOF;
    }
    if (ok < 0) {
        PlaybackLogger::printStringError(ok, "av_read_frame failed:");
        av_packet_free(&pkt);
        return DemuxerErrorType::ReadPacketError;
    }

    //av_packet_free(&pkt);
    return NoError;
}

// bool Demuxer::seek(double timestamp)
// {
//     int streamIndex = getAudioStreamIndex();

//     AVStream *stream = m_fmtCtx->streams[streamIndex];

//     // Convert milliseconds to stream timebase units
//     int64_t ts = static_cast<int64_t>(
//         (timestamp / 1000.0) * stream->time_base.den / stream->time_base.num
//         );

//     int ok = av_seek_frame(m_fmtCtx, streamIndex, ts, AVSEEK_FLAG_ANY);
//     if (ok < 0) {
//         PlaybackLogger::printStringError(ok, "Failed to seek:");
//         return false;
//     }

//     return true;
// }

void Demuxer::open(VideoState *state, const QString &file)
{
    if (avformat_open_input(&fc, file.toUtf8().constData(), nullptr, nullptr) < 0) {
        qCritical() << "Failed to open media file:" << file;
        return;
    }

    state->fileName = file;
    state->fc = fc;

    if (avformat_find_stream_info(fc, nullptr) < 0) {
        avformat_close_input(&fc);
        return;
    }

    for (unsigned int i = 0; i < fc->nb_streams; i++) {
        AVStream *avstream = fc->streams[i];
        if (!checkStreamType(avstream->codecpar)) {
            continue;
        }

        // Discard unwanted streams
        // avstream->discard = AVDISCARD_ALL;

        if (avstream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            // avstream->discard = AVDISCARD_DEFAULT;
            state->video_st = avstream;
            state->video_st_index = i;
            st_index[AVMEDIA_TYPE_VIDEO] = i;
        } else if (avstream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            // avstream->discard = AVDISCARD_DEFAULT;
            state->audio_st = avstream;
            state->audio_st_index = i;
            st_index[AVMEDIA_TYPE_AUDIO] = i;
        }

        qDebug().noquote() << "Found" << typeToString(avstream->codecpar) <<  "stream";
    }

    //av_dump_format(m_fmtCtx, 0, file.toUtf8().constData(), 0);
}

int Demuxer::getVideoStreamIndex()
{
    return st_index[AVMEDIA_TYPE_VIDEO];
}

int Demuxer::getAudioStreamIndex()
{
    return st_index[AVMEDIA_TYPE_AUDIO];
}

void Demuxer::seek(VideoState *state, double target)
{
    int64_t seek_target = av_rescale_q(target, AVRational{1,1}, state->video_st->time_base);
    av_seek_frame(fc, state->video_st_index, seek_target, AVSEEK_FLAG_ANY);
}

bool Demuxer::checkStreamType(AVCodecParameters *codecpar)
{
    AVMediaType type = codecpar->codec_type;

    switch (type) {
    case AVMEDIA_TYPE_VIDEO:
        return codecpar->codec_id != AV_CODEC_ID_PNG;
        break;
    case AVMEDIA_TYPE_AUDIO:
        return true;
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        return true;
        break;
    default:
        return false;
        break;
    }
}
