import QtQuick
import QtMultimedia
import com.systems.inspector 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: "Android HAL Inspector"
    color: "black"

    CaptureSession {
        camera: Camera {
            id: camera
            active: true
        }
        videoOutput: output
    }

    VideoOutput {
        id: output
        anchors.fill: parent

        // This 'sink' property comes from VideoOutput
        // We pass it to our C++ FrameProcessor
        FrameProcessor {
            videoSink: output.videoSink
        }
    }

    // Debug Overlay
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: 100
        color: "#80000000"

        Column {
            anchors.centerIn: parent
            Text {
                text: "HAL Inspection Mode"
                color: "#00ff00"
                font.bold: true
            }
            Text {
                text: "Check Qt Creator 'Application Output' for Frame Data"
                color: "white"
            }
        }
    }
}
