#ifndef PLAYBACKLOGGER_H
#define PLAYBACKLOGGER_H

#include <QDebug>

class PlaybackLogger
{
public:
    PlaybackLogger();

    static void printStringError(int returnCode, const QString &message);
};

#endif // PLAYBACKLOGGER_H
