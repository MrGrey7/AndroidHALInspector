#include "FrameProcessor.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <opencv2/imgproc.hpp>

FrameProcessor::FrameProcessor(QObject *parent) : QObject(parent) {
    setActive(true);
}

QVideoSink* FrameProcessor::videoSink() const { return m_sink; }

void FrameProcessor::setVideoSink(QVideoSink* sink) {
    if (m_sink == sink) return;
    m_sink = sink;
    connect(m_sink, &QVideoSink::videoFrameChanged, this, &FrameProcessor::processFrame);
    emit videoSinkChanged();
}

bool FrameProcessor::active() const { return m_active; }
void FrameProcessor::setActive(bool active) {
    if (m_active == active) return;
    m_active = active;

    if (m_active && !m_classifierLoaded) {
        loadClassifier();
    }

    // Reset state when disabling
    if (!m_active) {
        m_motionLevel = 0.0;
        emit motionEnergyChanged();
        m_prevFrame.release(); // Free memory
    }
    emit activeChanged();
}

double FrameProcessor::motionEnergy() const { return m_motionLevel; }

void FrameProcessor::setLoggingEnabled(bool enabled) {
    if (m_loggingEnabled == enabled) return;
    m_loggingEnabled = enabled;
    emit loggingEnabledChanged();
}

void FrameProcessor::processFrame(const QVideoFrame& frame) {
    // 1. SILENCER CHECK: If inactive, do absolutely nothing. Zero CPU usage.
    if (!m_active) return;

    if (!frame.isValid()) return;

    // Map Hardware Buffer
    QVideoFrame clone = frame;
    if (clone.map(QVideoFrame::ReadOnly)) {

        // Android Camera usually outputs NV12 or NV21.
        // Both have the Y-Plane (Luma) at the start, which is all we need for motion.
        if (clone.pixelFormat() == QVideoFrameFormat::Format_NV12 ||
            clone.pixelFormat() == QVideoFrameFormat::Format_NV21) {

            // 2. Wrap Y-Plane (Zero Copy)
            cv::Mat currentY(clone.height(), clone.width(), CV_8UC1,
                             (void*)clone.bits(0), clone.bytesPerLine(0));

            // 3. OPTIMIZATION: Downscale
            // Processing 1080p is slow. 320x240 is fast.
            cv::Mat smallUnrotatedFrame;
            // Calculate aspect-ratio correct scale
            double scale = 320.0 / clone.width();
            cv::resize(currentY, smallUnrotatedFrame, cv::Size(), scale, scale, cv::INTER_LINEAR);


            cv::Mat rotatedSmallFrame;
            cv::rotate(smallUnrotatedFrame, rotatedSmallFrame, cv::ROTATE_90_CLOCKWISE);

            // 3. Equalize Histogram (Improves detection in bad lighting)
            cv::equalizeHist(rotatedSmallFrame, rotatedSmallFrame);

            // 4. Detect Faces
            std::vector<cv::Rect> faces;
            if (m_classifierLoaded) {
                // Relaxed parameters for testing: scale 1.2, neighbors 2
                m_faceClassifier.detectMultiScale(rotatedSmallFrame, faces, 1.2, 2, 0, cv::Size(30, 30));
            }

            // 3. Update UI (Systems Logic)
            if (!faces.empty()) {
                // Get the first face
                cv::Rect r = faces[0];

                // Normalize coordinates to 0.0 - 1.0 range based on the SMALL frame size
                // This makes the UI code resolution-independent
                qreal nx = (qreal)r.x / rotatedSmallFrame.cols;
                qreal ny = (qreal)r.y / rotatedSmallFrame.rows;
                qreal nw = (qreal)r.width / rotatedSmallFrame.cols;
                qreal nh = (qreal)r.height / rotatedSmallFrame.rows;

                m_faceRect = QRectF(nx, ny, nw, nh);
            } else {
                // Reset if lost
                m_faceRect = QRectF(0, 0, 0, 0);
            }

            // Always emit to update UI (clearing the box if no face)
            emit faceRectChanged();
            emit facesDetectedChanged();



            // 4. Motion Detection Logic
            if (!m_prevFrame.empty()) {
                cv::Mat diff;
                cv::absdiff(rotatedSmallFrame, m_prevFrame, diff);
                cv::threshold(diff, diff, 30, 255, cv::THRESH_BINARY);

                int changedPixels = cv::countNonZero(diff);
                double newEnergy = (double)changedPixels / (diff.rows * diff.cols);

                // Update UI property
                // Smooth the value slightly for better visuals (Simple Low Pass Filter)
                m_motionLevel = (m_motionLevel * 0.7) + (newEnergy * 0.3);
                emit motionEnergyChanged();

                // Log only if significant motion (Reduce spam) & logging is enabled
                if (m_loggingEnabled && newEnergy > 0.05) {
                    qDebug() << "[Sentry] Motion Detected:" << newEnergy;
                }
            }

            // --- SYSTEM STATS LOGIC ---
            // Only calculate and print Luma stats if Logging is Enabled
            // This saves console spam
            static int frameCount = 0;
            if (m_loggingEnabled && frameCount++ % 60 == 0) {
                cv::Scalar meanVal = cv::mean(currentY); // Only calc if needed
                qDebug() << "[System] Frame:" << frame.width() << "x" << frame.height()
                         << "| Avg Luma:" << meanVal[0]
                         << "| Handle:" << frame.handleType();
            }

            // Store for next frame (Deep Copy required)
            rotatedSmallFrame.copyTo(m_prevFrame);
        }
        clone.unmap();
    }
}

void FrameProcessor::loadClassifier() {
    if (m_classifierLoaded) return;

    // 1. Define the internal path (Inside APK)
    // Note: The path depends on your CMake resource structure.
    // It usually matches the source path structure.
    QString resourcePath = ":/com/systems/inspector/content/assets/haarcascade_frontalface_default.xml";


    // 2. Define the target path (App Private Storage)
    QString targetPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + "/haarcascade_frontalface_default.xml";

    // Ensure directory exists
    QDir dir = QFileInfo(targetPath).absoluteDir();
    if (!dir.exists()) dir.mkpath(".");

    // 3. COPY System: Extract if not exists or if size differs (Update)
    QFile resourceFile(resourcePath);
    if (!resourceFile.exists()) {
        qWarning() << "[System] Model file NOT FOUND in resources:" << resourcePath;
        return;
    }

    // Always try to copy to ensure we have the file on disk
    if (QFile::exists(targetPath)) QFile::remove(targetPath);

    if (resourceFile.copy(targetPath)) {
        // 4. Set permissions so OpenCV can read it
        QFile::setPermissions(targetPath, QFile::ReadOwner | QFile::WriteOwner);

        // 5. Initialize OpenCV with the REAL file path
        if (m_faceClassifier.load(targetPath.toStdString())) {
            m_classifierLoaded = true;
            qDebug() << "[System] Face Classifier loaded successfully from:" << targetPath;
        } else {
            qWarning() << "[System] OpenCV failed to load XML from:" << targetPath;
        }
    } else {
        qWarning() << "[System] Failed to copy model to cache:" << targetPath;
    }
}
