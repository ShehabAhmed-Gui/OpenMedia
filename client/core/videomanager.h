#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include "settings.h"
#include <QObject>

class VideoManager : public QObject
{
    Q_OBJECT
public:
    explicit VideoManager(QSharedPointer<Settings> settings,
                          QObject *parent = nullptr);

    Q_PROPERTY(Loop loopState READ loopState WRITE setLoopState NOTIFY loopStateChanged FINAL)

    enum Loop {
        Disabled = 0,
        Enabled = 1
    };

    Loop loopState() const;
    void setLoopState(Loop newLoopState);

signals:
    void loopStateChanged();

private:
    Loop m_loopState = Disabled;

    QSharedPointer<Settings> m_settings;
};

#endif // VIDEOMANAGER_H
