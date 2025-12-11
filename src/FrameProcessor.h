#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <QRectF>
#include <QVariantList>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

class FrameProcessor : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

    // Controls
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged) // Master Switch
    Q_PROPERTY(bool loggingEnabled READ loggingEnabled WRITE setLoggingEnabled NOTIFY loggingEnabledChanged)
    Q_PROPERTY(bool faceDetectionEnabled READ faceDetectionEnabled WRITE setFaceDetectionEnabled NOTIFY faceDetectionEnabledChanged)
    Q_PROPERTY(bool objectDetectionEnabled READ objectDetectionEnabled WRITE setObjectDetectionEnabled NOTIFY objectDetectionEnabledChanged)

    // Data Outputs
    Q_PROPERTY(double motionEnergy READ motionEnergy NOTIFY motionEnergyChanged)
    Q_PROPERTY(QRectF faceRect READ faceRect NOTIFY faceRectChanged)
    Q_PROPERTY(QVariantList detectedRects READ detectedRects NOTIFY detectionsChanged)
    Q_PROPERTY(QVariantList detectedLabels READ detectedLabels NOTIFY detectionsChanged)

public:
    explicit FrameProcessor(QObject *parent = nullptr);

    QVideoSink* videoSink() const;
    void setVideoSink(QVideoSink* sink);

    bool active() const;
    void setActive(bool active);

    bool loggingEnabled() const { return m_loggingEnabled; }
    void setLoggingEnabled(bool enabled);

    // NEW Getters/Setters
    bool faceDetectionEnabled() const { return m_faceDetectionEnabled; }
    void setFaceDetectionEnabled(bool enabled);

    bool objectDetectionEnabled() const { return m_objectDetectionEnabled; }
    void setObjectDetectionEnabled(bool enabled);

    double motionEnergy() const { return m_motionLevel; }
    QRectF faceRect() const { return m_faceRect; }
    QVariantList detectedRects() const { return m_detectedRects; }
    QVariantList detectedLabels() const { return m_detectedLabels; }

public slots:
    void processFrame(const QVideoFrame& frame);

signals:
    void videoSinkChanged();
    void activeChanged();
    void loggingEnabledChanged();
    void motionEnergyChanged();
    void faceRectChanged();
    void detectionsChanged();
    void faceDetectionEnabledChanged();
    void objectDetectionEnabledChanged();

private:
    QVideoSink* m_sink = nullptr;

    // Flags
    bool m_active = true;
    bool m_loggingEnabled = true;
    bool m_faceDetectionEnabled = true;
    bool m_objectDetectionEnabled = true;

    // State
    double m_motionLevel = 0.0;
    QRectF m_faceRect;
    QVariantList m_detectedRects;
    QVariantList m_detectedLabels;

    // CV Members
    cv::Mat m_prevFrame;
    cv::CascadeClassifier m_faceClassifier;
    bool m_classifierLoaded = false;
    cv::dnn::Net m_net;
    bool m_netLoaded = false;
    const std::vector<QString> m_classNames = {
        "Background", "Plane", "Bicycle", "Bird", "Boat",
        "Bottle", "Bus", "Car", "Cat", "Chair", "Cow",
        "Table", "Dog", "Horse", "Motorbike", "Person",
        "PottedPlant", "Sheep", "Sofa", "Train", "TV"
    };

    void loadClassifier();
    void loadNetwork();
    QString extractAsset(const QString& name);
};

#endif // FRAMEPROCESSOR_H
