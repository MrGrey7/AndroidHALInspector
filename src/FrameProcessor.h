#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <QVariantList>
#include <opencv2/objdetect.hpp>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

class FrameProcessor : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

    // NEW: Controls
    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(double motionEnergy READ motionEnergy NOTIFY motionEnergyChanged)
    Q_PROPERTY(bool loggingEnabled READ loggingEnabled WRITE setLoggingEnabled NOTIFY loggingEnabledChanged)
    Q_PROPERTY(int facesDetected READ facesDetected NOTIFY facesDetectedChanged)
    Q_PROPERTY(QRectF faceRect READ faceRect NOTIFY faceRectChanged)
    Q_PROPERTY(QVariantList detectedRects READ detectedRects NOTIFY detectionsChanged)
    Q_PROPERTY(QVariantList detectedLabels READ detectedLabels NOTIFY detectionsChanged)

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

    QVariantList detectedRects() const { return m_detectedRects; }
    QVariantList detectedLabels() const { return m_detectedLabels; }
public slots:
    void processFrame(const QVideoFrame& frame);

signals:
    void videoSinkChanged();
    void activeChanged();
    void motionEnergyChanged();
    void loggingEnabledChanged();
    void facesDetectedChanged();
    void faceRectChanged();
    void detectionsChanged();

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


    // DNN state
    cv::dnn::Net m_net;
    bool m_netLoaded = false;
    QVariantList m_detectedRects;
    QVariantList m_detectedLabels;

    // Helper to extract assets
    QString extractAsset(const QString& name);
    void loadNetwork();

    // Class names lookup (0=Background, 1=Plane, 2=Bicycle...)
    const std::vector<QString> m_classNames = {
        "Background", "Plane", "Bicycle", "Bird", "Boat",
        "Bottle", "Bus", "Car", "Cat", "Chair", "Cow",
        "Table", "Dog", "Horse", "Motorbike", "Person",
        "PottedPlant", "Sheep", "Sofa", "Train", "TV"
    };
};

#endif // FRAMEPROCESSOR_H
