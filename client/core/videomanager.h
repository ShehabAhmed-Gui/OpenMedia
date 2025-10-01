#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QObject>

class VideoManager : public QObject
{
    Q_OBJECT
public:
    explicit VideoManager(QObject *parent = nullptr);

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
};

#endif // VIDEOMANAGER_H
