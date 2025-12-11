#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <opencv2/objdetect.hpp>
#include <opencv2/core.hpp>

class FrameProcessor : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

    // NEW: Controls
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(double motionEnergy READ motionEnergy NOTIFY motionEnergyChanged)
    Q_PROPERTY(bool loggingEnabled READ loggingEnabled WRITE setLoggingEnabled NOTIFY loggingEnabledChanged)
    Q_PROPERTY(int facesDetected READ facesDetected NOTIFY facesDetectedChanged)
    Q_PROPERTY(QRectF faceRect READ faceRect NOTIFY faceRectChanged)

public:
    explicit FrameProcessor(QObject *parent = nullptr);

    QVideoSink* videoSink() const;
    void setVideoSink(QVideoSink* sink);

    bool active() const;
    void setActive(bool active);

    double motionEnergy() const;

    bool loggingEnabled() const { return m_loggingEnabled; }
    void setLoggingEnabled(bool enabled);

    int facesDetected() const { return m_facesDetected; }
    QRectF faceRect() const { return m_faceRect; }
public slots:
    void processFrame(const QVideoFrame& frame);

signals:
    void videoSinkChanged();
    void activeChanged();
    void motionEnergyChanged();
    void loggingEnabledChanged();
    void facesDetectedChanged();
    void faceRectChanged();

private:
    QVideoSink* m_sink = nullptr;
    bool m_active = false;
    double m_motionLevel = 0.0;
    bool m_loggingEnabled = true;
    // Sentry Mode State
    cv::Mat m_prevFrame;
    // Face Recognition
    int m_facesDetected = 0;
    QRectF m_faceRect;
    // OpenCV Classifier
    cv::CascadeClassifier m_faceClassifier;
    bool m_classifierLoaded = false;


    // Helper to extract XML from APK to Disk
    void loadClassifier();
};

#endif // FRAMEPROCESSOR_H
