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
            loggingEnabled: true
            faceDetectionEnabled: true
            objectDetectionEnabled: true
        }

        // --- FACE TRACKING BOX ---
        Rectangle {
            id: faceBox
            x: processor.faceRect.x * output.width
            y: processor.faceRect.y * output.height
            width: processor.faceRect.width * output.width
            height: processor.faceRect.height * output.height
            color: "transparent"
            border.color: "#00AA00"
            border.width: 3
            visible: width > 0 && processor.active && processor.faceDetectionEnabled

            Text {
                text: "FACE"
                color: "#00AA00"
                font.bold: true
                font.pixelSize: 14
                anchors.bottom: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        // --- OBJECT DETECTION OVERLAY ---
        Item {
            anchors.fill: parent
            visible: processor.active && processor.objectDetectionEnabled
            Repeater {
                model: processor.detectedRects.length
                delegate: Rectangle {
                    property rect r: processor.detectedRects[index]
                    property string l: processor.detectedLabels[index]
                    x: r.x * output.width
                    y: r.y * output.height
                    width: r.width * output.width
                    height: r.height * output.height
                    color: "transparent"
                    border.color: "#55FFFF"
                    border.width: 3
                    Rectangle {
                        color: "#55FFFF"
                        width: labelText.width + 10
                        height: labelText.height + 4
                        anchors.bottom: parent.top
                        anchors.left: parent.left
                        Text {
                            id: labelText
                            anchors.centerIn: parent
                            text: l
                            font.bold: true
                            color: "black"
                        }
                    }
                }
            }
        }
    }

    // --- HUD Overlay ---
    Rectangle {
        anchors.fill: parent
        color: "transparent"

        // 1. Motion Energy Bar (Labeled)
        Item {
            width: 50
            height: 300
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 20
            visible: processor.active

            Rectangle {
                id: barBg
                width: 20
                height: parent.height
                color: "#80000000"
                border.color: "white"
                anchors.centerIn: parent

                Rectangle {
                    width: parent.width - 4
                    height: (parent.height - 4) * Math.min(processor.motionEnergy * 5, 1.0)
                    color: processor.motionEnergy > 0.1 ? "red" : "#00ff00"
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottomMargin: 2
                }
            }

            // Explicit Label
            Text {
                text: "Motion\nEnergy"
                color: "white"
                font.bold: true
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                anchors.top: barBg.bottom
                anchors.topMargin: 5
                anchors.horizontalCenter: barBg.horizontalCenter
            }
        }

        // 2. Control Panel
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 30 // Lifted
            width: parent.width
            height: 200
            color: "#CC000000"

            Column {
                anchors.centerIn: parent
                spacing: 5

                // Master Switch
                Switch {
                    text: "MASTER: Processing"
                    checked: processor.active
                    onCheckedChanged: processor.active = checked
                    contentItem: Text { text: parent.text; color: parent.checked ? "#00ff00" : "gray"; font.bold: true; leftPadding: parent.indicator.width + 10; verticalAlignment: Text.AlignVCenter }
                }

                // Sub-Features
                Switch {
                    text: "Face Detection (Haar)"
                    checked: processor.faceDetectionEnabled
                    enabled: processor.active
                    onCheckedChanged: processor.faceDetectionEnabled = checked
                    contentItem: Text { text: parent.text; color: parent.checked ? "white" : "gray"; leftPadding: parent.indicator.width + 10; verticalAlignment: Text.AlignVCenter }
                }

                Switch {
                    text: "Object Detection (DNN)"
                    checked: processor.objectDetectionEnabled
                    enabled: processor.active
                    onCheckedChanged: processor.objectDetectionEnabled = checked
                    contentItem: Text { text: parent.text; color: parent.checked ? "white" : "gray"; leftPadding: parent.indicator.width + 10; verticalAlignment: Text.AlignVCenter }
                }

                Switch {
                    text: "Console Logging"
                    checked: processor.loggingEnabled
                    onCheckedChanged: processor.loggingEnabled = checked
                    contentItem: Text { text: parent.text; color: parent.checked ? "#00aaff" : "gray"; leftPadding: parent.indicator.width + 10; verticalAlignment: Text.AlignVCenter }
                }
            }
        }
    }
}
