#include "FrameProcessor.h"

#include <QDebug>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

FrameProcessor::FrameProcessor(QObject *parent)
    : QObject(parent)
{
}

QVideoSink* FrameProcessor::videoSink() const {
    return m_sink;
}

void FrameProcessor::setVideoSink(QVideoSink* sink) {
    if (m_sink == sink) return;
    m_sink = sink;
    connect(m_sink, &QVideoSink::videoFrameChanged, this, &FrameProcessor::processFrame);
    emit videoSinkChanged();
}

void FrameProcessor::processFrame(const QVideoFrame& frame) {
    static int frameCount = 0;
    // Log every 60th frame to keep console readable
    if (frameCount++ % 60 != 0) return;

    if (!frame.isValid()) return;

    QVideoFrame clone = frame;

    if (clone.map(QVideoFrame::ReadOnly)) {

        QVideoFrameFormat::PixelFormat fmt = clone.pixelFormat();

        // Android HALs usually output NV21 (Legacy) or NV12 (Modern).
        // For Luma extraction, the memory layout of Plane 0 is identical.
        if (fmt == QVideoFrameFormat::Format_NV21 ||
            fmt == QVideoFrameFormat::Format_NV12) {

            // 1. Wrap the Y-Plane (Luma) in a cv::Mat. ZERO COPY.
            // NV21/NV12 Structure:
            // [ Y Plane (Width x Height) ] <-- We are looking here
            // [ UV Plane (Width x Height / 2) ]

            cv::Mat yPlane(clone.height(), clone.width(), CV_8UC1,
                           (void*)clone.bits(0), clone.bytesPerLine(0));

            // 2. Simple CV Operation: Calculate average brightness
            // This proves we have read access to the hardware buffer
            cv::Scalar meanVal = cv::mean(yPlane);

            const char* fmtStr = (fmt == QVideoFrameFormat::Format_NV21) ? "NV21" : "NV12";

            qDebug() << "[System] Frame:" << frame.width() << "x" << frame.height()
                     << "| Format:" << fmtStr
                     << "| Handle:" << frame.handleType()
                     << "| Avg Luma:" << meanVal[0];
        }
        else {
            qDebug() << "[System] Unhandled Format:" << fmt;
        }

        clone.unmap();
    } else {
        qDebug() << "[System] Failed to map hardware buffer. (Secure Memory?)";
    }
}
