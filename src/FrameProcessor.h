#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>

class FrameProcessor : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

public:
    explicit FrameProcessor(QObject *parent = nullptr);

    QVideoSink* videoSink() const;
    void setVideoSink(QVideoSink* sink);

public slots:
    void processFrame(const QVideoFrame& frame);

signals:
    void videoSinkChanged();

private:
    QVideoSink* m_sink = nullptr;
};

#endif // FRAMEPROCESSOR_H
