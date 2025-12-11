#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <opencv2/core.hpp> // Keep OpenCV types in header if used as members

class FrameProcessor : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

    // NEW: Controls
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(double motionEnergy READ motionEnergy NOTIFY motionEnergyChanged)
    Q_PROPERTY(bool loggingEnabled READ loggingEnabled WRITE setLoggingEnabled NOTIFY loggingEnabledChanged)

public:
    explicit FrameProcessor(QObject *parent = nullptr);

    QVideoSink* videoSink() const;
    void setVideoSink(QVideoSink* sink);

    bool active() const;
    void setActive(bool active);

    double motionEnergy() const;

    bool loggingEnabled() const { return m_loggingEnabled; }
    void setLoggingEnabled(bool enabled);

public slots:
    void processFrame(const QVideoFrame& frame);

signals:
    void videoSinkChanged();
    void activeChanged();
    void motionEnergyChanged();
    void loggingEnabledChanged();

private:
    QVideoSink* m_sink = nullptr;
    bool m_active = true;
    double m_motionLevel = 0.0;
    bool m_loggingEnabled = true;
    // Sentry Mode State
    cv::Mat m_prevFrame;
};

#endif // FRAMEPROCESSOR_H
