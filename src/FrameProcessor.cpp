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

    if (m_active && !m_netLoaded) {
        loadNetwork();
    }

    // Reset state when disabling
    if (!m_active) {
        m_motionLevel = 0.0;
        emit motionEnergyChanged();
        m_prevFrame.release(); // Free memory
    }
    emit activeChanged();
}

QString FrameProcessor::extractAsset(const QString& name) {
    QString resourcePath = ":/com/systems/inspector/content/assets/" + name;
    QString targetPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + name;

    // Only copy if missing (Simple caching)
    if (!QFile::exists(targetPath)) {
        QFile::copy(resourcePath, targetPath);
        QFile::setPermissions(targetPath, QFile::ReadOwner | QFile::WriteOwner);
    }
    return targetPath;
}

void FrameProcessor::loadNetwork() {
    if (m_netLoaded) return;

    QString protoPath = extractAsset("MobileNetSSD_deploy.prototxt");
    QString modelPath = extractAsset("MobileNetSSD_deploy.caffemodel");

    try {
        m_net = cv::dnn::readNetFromCaffe(protoPath.toStdString(), modelPath.toStdString());

        // SYSTEMS OPTIMIZATION: Use OpenCL if available (UMat backend)
        m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU); // Or OPENCL if stable

        m_netLoaded = !m_net.empty();
        if (m_netLoaded) qDebug() << "[System] DNN Loaded Successfully.";
    } catch (const cv::Exception& e) {
        qWarning() << "[System] DNN Load Failed:" << e.what();
    }
}

void FrameProcessor::setLoggingEnabled(bool enabled) {
    if (m_loggingEnabled == enabled) return;
    m_loggingEnabled = enabled;
    emit loggingEnabledChanged();
}

void FrameProcessor::setFaceDetectionEnabled(bool enabled) {
    if (m_faceDetectionEnabled == enabled) return;
    m_faceDetectionEnabled = enabled;
    if (m_faceDetectionEnabled) loadClassifier(); // Lazy load
    else {
        m_faceRect = QRectF(0,0,0,0);
        emit faceRectChanged();
    }
    emit faceDetectionEnabledChanged();
}

void FrameProcessor::setObjectDetectionEnabled(bool enabled) {
    if (m_objectDetectionEnabled == enabled) return;
    m_objectDetectionEnabled = enabled;
    if (m_objectDetectionEnabled) loadNetwork(); // Lazy load
    else {
        m_detectedRects.clear();
        m_detectedLabels.clear();
        emit detectionsChanged();
    }
    emit objectDetectionEnabledChanged();
}


void FrameProcessor::processFrame(const QVideoFrame& frame) {
    if (!m_active) return;
    if (!frame.isValid()) return;

    // Throttle counters
    static int frameCount = 0;
    frameCount++;

    QVideoFrame clone = frame;
    if (clone.map(QVideoFrame::ReadOnly)) {
        if (clone.pixelFormat() == QVideoFrameFormat::Format_NV12 ||
            clone.pixelFormat() == QVideoFrameFormat::Format_NV21) {

            // --- 1. SHARED PRE-PROCESSING (Do once) ---

            // A. Full Size Y-Plane (For Stats)
            cv::Mat fullY(clone.height(), clone.width(), CV_8UC1,
                          (void*)clone.bits(0), clone.bytesPerLine(0));

            // B. Downscaled & Rotated Y-Plane (For Face & Motion)
            // Use specific size to ensure consistency
            cv::Mat smallY;
            double scale = 320.0 / clone.width(); // e.g. 1920 -> 320
            cv::resize(fullY, smallY, cv::Size(), scale, scale, cv::INTER_NEAREST); // Nearest is faster

            cv::Mat rotatedY;
            cv::rotate(smallY, rotatedY, cv::ROTATE_90_CLOCKWISE);


            // --- 2. MOTION DETECTION (Fast - Every Frame) ---
            if (!m_prevFrame.empty() && m_prevFrame.size() == rotatedY.size()) {
                cv::Mat diff;
                cv::absdiff(rotatedY, m_prevFrame, diff);
                cv::threshold(diff, diff, 30, 255, cv::THRESH_BINARY);

                // Quick check: count non-zero is fast on small images
                int changedPixels = cv::countNonZero(diff);
                double newEnergy = (double)changedPixels / (diff.rows * diff.cols);

                m_motionLevel = (m_motionLevel * 0.7) + (newEnergy * 0.3);
                emit motionEnergyChanged();

                if (m_loggingEnabled && newEnergy > 0.05) {
                    qDebug() << "[Sentry] Motion Detected:" << newEnergy;
                }
            }
            rotatedY.copyTo(m_prevFrame); // Deep copy for next loop


            // --- 3. FACE DETECTION (Medium - Every 5 frames) ---
            if (m_faceDetectionEnabled && frameCount % 5 == 0 && m_classifierLoaded) {
                cv::Mat eqFrame;
                cv::equalizeHist(rotatedY, eqFrame);

                std::vector<cv::Rect> faces;
                m_faceClassifier.detectMultiScale(eqFrame, faces, 1.2, 2, 0, cv::Size(30, 30));

                if (!faces.empty()) {
                    cv::Rect r = faces[0];
                    qreal nx = (qreal)r.x / rotatedY.cols;
                    qreal ny = (qreal)r.y / rotatedY.rows;
                    qreal nw = (qreal)r.width / rotatedY.cols;
                    qreal nh = (qreal)r.height / rotatedY.rows;
                    m_faceRect = QRectF(nx, ny, nw, nh);
                } else {
                    m_faceRect = QRectF(0, 0, 0, 0);
                }
                emit faceRectChanged();
                // If you had facesDetected property, update it here
            }


            // --- 4. OBJECT DETECTION (Heavy - Every 30 frames) ---
            if (m_objectDetectionEnabled && frameCount % 30 == 0 && m_netLoaded) {
                // DNN needs Color (BGR). Convert only the small frame!
                // Don't convert 1080p -> BGR, that's slow.
                // We reconstruct BGR from the small Y (gray) ? No, we need color.
                // Re-extract small color from source NV12.

                if (frameCount % 30 == 0 && m_netLoaded) {
                    // 1. Get Full Color (Necessary for DNN)
                    cv::Mat fullYUV(clone.height() * 3 / 2, clone.width(), CV_8UC1, (void*)clone.bits(0), clone.bytesPerLine(0));
                    cv::Mat fullBGR;
                    cv::cvtColor(fullYUV, fullBGR, cv::COLOR_YUV2BGR_NV21);

                    // 2. OPTIMIZATION: Resize FIRST, then Rotate
                    // Don't rotate 1080p. Resize to network size (300x300) first.
                    cv::Mat smallBGR;
                    cv::resize(fullBGR, smallBGR, cv::Size(300, 300)); // Squashes it, but SSD handles this ok

                    // 3. Rotate the small image (Fast!)
                    cv::Mat rotatedBGR;
                    cv::rotate(smallBGR, rotatedBGR, cv::ROTATE_90_CLOCKWISE);

                    // 4. Create Blob
                    // Note: We don't need to resize in blobFromImage anymore, we did it above.
                    cv::Mat blob = cv::dnn::blobFromImage(rotatedBGR, 0.007843, cv::Size(300, 300),
                                                          cv::Scalar(127.5, 127.5, 127.5), false, false);

                    m_net.setInput(blob);
                    cv::Mat detection = m_net.forward();

                    QVariantList newRects;
                    QVariantList newLabels;

                    cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

                    for (int i = 0; i < detectionMat.rows; i++) {
                        float confidence = detectionMat.at<float>(i, 2);

                        if (confidence > 0.4) {
                            int classId = (int)(detectionMat.at<float>(i, 1));

                            // SSD outputs normalized coordinates (0.0 - 1.0)
                            float xLeft = detectionMat.at<float>(i, 3);
                            float yTop = detectionMat.at<float>(i, 4);
                            float xRight = detectionMat.at<float>(i, 5);
                            float yBottom = detectionMat.at<float>(i, 6);

                            // Clamp
                            xLeft = std::max(0.0f, xLeft);
                            yTop = std::max(0.0f, yTop);

                            // Since we rotated the input, these coordinates map to the PORTRAIT screen.
                            newRects.append(QRectF(xLeft, yTop, xRight - xLeft, yBottom - yTop));

                            QString label = "Unknown";
                            if (classId >= 0 && classId < m_classNames.size()) {
                                label = m_classNames[classId] + " " + QString::number(int(confidence * 100)) + "%";
                            }
                            newLabels.append(label);
                        }
                    }
                    m_detectedRects = newRects;
                    m_detectedLabels = newLabels;
                    emit detectionsChanged();
                }
            }

            // --- 5. LOGGING ---
            if (m_loggingEnabled && frameCount % 60 == 0) {
                cv::Scalar meanVal = cv::mean(fullY);
                qDebug() << "[System] Frame:" << frame.width() << "x" << frame.height()
                         << "| Avg Luma:" << meanVal[0]
                         << "| Handle:" << frame.handleType();
            }
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
