#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <QDebug>

class FrameProcessor : public QObject {
    Q_OBJECT
    // This property allows QML to pass the 'videoSink' to us
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

public:
    explicit FrameProcessor(QObject *parent = nullptr) : QObject(parent) {}

    QVideoSink* videoSink() const { return m_sink; }

    void setVideoSink(QVideoSink* sink) {
        if (m_sink == sink) return;
        m_sink = sink;

        // Hook into the stream (The "HAL" handshake)
        // Whenever a new frame arrives from the Camera HAL, this function runs.
        connect(m_sink, &QVideoSink::videoFrameChanged, this, &FrameProcessor::processFrame);
        emit videoSinkChanged();
    }

public slots:
    void processFrame(const QVideoFrame& frame) {
        static int counter = 0;
        // Don't spam logs (60fps is fast), print every 60th frame
        if (counter++ % 60 == 0) {
            qDebug() << "--- HAL Frame Inspection ---";
            qDebug() << "Size:" << frame.width() << "x" << frame.height();

            // The Pixel Format (e.g., NV12, YUV420P)
            // If you see 'Format_SamplerExternalOES', that's a raw hardware texture!
            qDebug() << "Format:" << frame.pixelFormat();

            // The Handle Type (Memory Location)
            // RhiTextureHandle = GPU VRAM (Zero Copy)
            // NoHandle = CPU RAM (System Memory)
            qDebug() << "Handle Type:" << frame.handleType();

            // Map Check: Can we read it with CPU?
            // If this is a hardware texture, mapping might fail or be slow.
            qDebug() << "Is Mappable:" << frame.isMapped();
        }
    }

signals:
    void videoSinkChanged();

private:
    QVideoSink* m_sink = nullptr;
};

#endif // FRAMEPROCESSOR_H
