#ifndef AUDIOBUFFERDEVICE_H
#define AUDIOBUFFERDEVICE_H

#include <QIODevice>
#include <QMutexLocker>
#include <qdebug.h>

class AudioBufferDevice : public QIODevice
{
public:
    explicit AudioBufferDevice(QObject *parent = nullptr)
        : QIODevice{parent}
    {

    }

    void start() { open(QIODevice::ReadOnly); };
    void stop() { close(); };

    void append(const QByteArray &pcm) {
        QMutexLocker lock(&m_mutex);
        m_buffer.append(pcm);
    }

protected:
    qint64 bytesAvailable() const override
    {
        QMutexLocker lock(&m_mutex);
        return m_buffer.size() + QIODevice::bytesAvailable();
    }

    qint64 readData(char *data, qint64 maxlen) override
    {
        QMutexLocker lock(&m_mutex);

        qint64 len = qMin<qint64>(maxlen, m_buffer.size());
        if (len > 0) {
            memcpy(data, m_buffer.constData(), len);
            m_buffer.remove(0, len);
        }

        return len;
    }
    qint64 writeData(const char *data, qint64 len) override
    {
        // We don't need this
        return 0;
    }

private:
    mutable QMutex m_mutex;
    QByteArray m_buffer;
};

#endif // AUDIOBUFFERDEVICE_H
