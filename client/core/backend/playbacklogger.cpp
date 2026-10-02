#include "playbacklogger.h"

extern "C" {
#include <libavutil/error.h>
}

PlaybackLogger::PlaybackLogger() {}

void PlaybackLogger::printStringError(int returnCode, const QString &message)
{
    char errbuf[128];
    av_strerror(returnCode, errbuf, sizeof(errbuf));

    qDebug().noquote() << message << errbuf;
}
