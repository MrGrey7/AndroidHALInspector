# Android HAL Inspector

![Toolkit: QT 6](https://img.shields.io/badge/Toolkit-QT%206-green?logo=qt&logoColor=white)
![Platform: Android](https://img.shields.io/badge/Platform-Android%20(NDK)-blue?logo=android)
![Status: Prototype](https://img.shields.io/badge/Status-Prototype-orange)

A systems engineering utility to inspect **Camera Hardware Abstraction Layer (HAL)** buffers on Android devices.

It bridges the gap between the high-level Qt Multimedia framework and the low-level Android Camera stack, allowing real-time inspection of hardware buffers, pixel formats, and memory handle types (e.g., `OES_external_image` vs `NV12`).

## 🎯 Engineering Goals

1.  **Zero-Copy Verification:** Verify that the video pipeline utilizes **Hardware Textures** (`RhiTextureHandle`) rather than CPU-bound copies.
2.  **Buffer Introspection:** Log raw `QVideoFrame` metadata to confirm format negotiation (NV12/YUV) and memory layout.
3.  **NDK Integration:** Demonstrate linking external native C++ libraries (OpenCV NDK) into the Qt/CMake build system for advanced signal processing.

## 🛠 Tech Stack

*   **Language:** C++17
*   **Framework:** Qt 6.10 (Quick / Multimedia)
*   **Build System:** CMake
*   **Target:** Android ARM64-v8a (API 34)

## 🔍 How it Works

The application creates a custom `FrameProcessor` class that hooks into the `QVideoSink`. Instead of simply rendering the frame, it intercepts the pointer to perform systems analysis:

```cpp
void processFrame(const QVideoFrame& frame) {
    // Check if we are holding a GPU Texture or System RAM
    qDebug() << "Handle Type:" << frame.handleType(); 
    
    // Check if the ISP gave us NV12 or YUV420P
    qDebug() << "Pixel Format:" << frame.pixelFormat();
}
```

## 🚀 Build Instructions

1.  **Requirements:**
    *   Qt 6.8+ (with Android ARM64 kit).
    *   Android SDK / NDK (r26+).
    *   (Optional) OpenCV Android SDK.

2.  **Build:**
    Open in Qt Creator and select the **Android Clang arm64-v8a** kit.

3.  **Permissions:**
    The app requires runtime Camera permissions. Grant them via Android Settings or the OS prompt.

## 📝 License
MIT
```