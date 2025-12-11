import QtQuick
import QtQuick.Controls 2.15
import QtMultimedia
import com.systems.inspector 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: "Rivian Sentry Test"
    color: "black"

    CaptureSession {
        camera: Camera { id: camera; active: true }
        videoOutput: output
    }

    VideoOutput {
        id: output
        anchors.fill: parent

        FrameProcessor {
            id: processor
            videoSink: output.videoSink
            active: true
            loggingEnabled: true // Default On
        }
    }

    // --- HUD Overlay ---
    Rectangle {
        anchors.fill: parent
        color: "transparent"

        // 1. Motion Energy Bar (Left Side)
        // Kept the same
        Rectangle {
            id: energyBarBackground
            width: 30
            height: 300
            color: "#80000000"
            border.color: "white"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 20
            visible: processor.active

            Rectangle {
                width: parent.width - 4
                height: (parent.height - 4) * Math.min(processor.motionEnergy * 5, 1.0)
                color: processor.motionEnergy > 0.1 ? "red" : "#00ff00"
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 2
            }
        }

        // 2. Control Panel (Lifted & Stacked)
        Rectangle {
            anchors.bottom: parent.bottom
            // LIFT: Move up by 50px to clear Android Nav Bar
            anchors.bottomMargin: 50
            width: parent.width
            height: 120 // Taller to fit 2 buttons
            color: "#CC000000"

            Column {
                anchors.centerIn: parent
                spacing: 10

                // Toggle 1: Sentry Processing (CPU load)
                Switch {
                    text: "Sentry Processing (CV)"
                    checked: processor.active
                    onCheckedChanged: processor.active = checked

                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "#00ff00" : "gray"
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: parent.indicator.width + 10
                    }
                }

                // Toggle 2: Console Logging (Noise control)
                Switch {
                    text: "Console Logging"
                    checked: processor.loggingEnabled
                    onCheckedChanged: processor.loggingEnabled = checked

                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? "#00aaff" : "gray"
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: parent.indicator.width + 10
                    }
                }
            }
        }
    }
}
