#include "FrameProcessor.h"
#include <QDebug>
#include <opencv2/imgproc.hpp>

FrameProcessor::FrameProcessor(QObject *parent) : QObject(parent) {}

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
            cv::Mat smallFrame;
            // Calculate aspect-ratio correct scale
            double scale = 320.0 / clone.width();
            cv::resize(currentY, smallFrame, cv::Size(), scale, scale, cv::INTER_LINEAR);

            // 4. Motion Detection Logic
            if (!m_prevFrame.empty()) {
                cv::Mat diff;
                cv::absdiff(smallFrame, m_prevFrame, diff);
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
            smallFrame.copyTo(m_prevFrame);
        }
        clone.unmap();
    }
}
