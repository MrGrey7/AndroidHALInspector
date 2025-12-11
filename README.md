# Android HAL Inspector & Perception Engine

![Toolkit: QT 6.10](https://img.shields.io/badge/Toolkit-QT%206.10-green?logo=qt&logoColor=white)
![Platform: Android](https://img.shields.io/badge/Platform-Android%20NDK-blue?logo=android)
![Engine: OpenCV](https://img.shields.io/badge/Engine-OpenCV%204.x-red?logo=opencv)
![Build: CMake](https://img.shields.io/badge/Build-CMake-orange?logo=cmake)

A systems engineering diagnostic tool designed to inspect **Android Camera Hardware Abstraction Layer (HAL)** buffers and demonstrate high-performance, zero-copy integration between Qt Multimedia and native C++ computer vision pipelines.

This project serves as a proof-of-concept for **Edge Perception Architectures**.

## 📸 System Dashboard

<img width="461" height="845" alt="image" src="https://github.com/user-attachments/assets/9f190e64-59df-4395-8cd5-d5fc2aef2d59" />
<img width="456" height="842" alt="image" src="https://github.com/user-attachments/assets/ad932b63-290e-42fb-aeaa-4f91b8ce4b8e" />



## 🧠 Engineering Highlights

### 1. Zero-Copy Hardware Access
*   **HAL Introspection:** Intercepts `QVideoFrame` handles to verify the usage of **Hardware Textures** (`RhiTextureHandle`) vs. System Memory.
*   **Memory Mapping:** Implements robust mapping of **NV12/NV21** buffers from the Image Signal Processor (ISP) directly into OpenCV `cv::Mat` wrappers without `memcpy`.
*   **Latency Analysis:** Verifies the "Zero-Copy" path critical for low-latency automotive and robotics video pipelines.

### 2. Embedded Perception Stack (OpenCV NDK)
*   **Deep Learning (DNN):** Integrated **MobileNet-SSD** (Caffe) via the OpenCV DNN module for real-time object detection (Person, Car, etc.).
*   **Heuristic Sentry Mode:** Implemented a low-power **Motion Detection** algorithm using frame differencing and dynamic thresholding on downscaled Luma planes.
*   **Sensor Orientation:** Solved the "Portrait/Landscape" sensor alignment mismatch by implementing a rotation pipeline before inference, ensuring coordinate space consistency between the neural network and the UI layer.

### 3. Systems Architecture
*   **Pipeline Throttling:** Implemented a hierarchical execution model to manage thermal constraints:
    *   **Motion Analysis:** 60 Hz (Every frame)
    *   **Face Detection:** 12 Hz (Every 5th frame)
    *   **DNN Inference:** 2 Hz (Every 30th frame)
*   **Asset Lifecycle Management:** Built a dynamic extraction engine to pull model weights (`.caffemodel`, `.xml`) from the APK's virtual file system (RCC) to the app's private storage at runtime, bypassing Android's asset sandboxing limits.
*   **Granular Control:** Exposed all subsystems via QML properties, allowing individual profiling of CPU cost for specific vision tasks.

## 🛠 Tech Stack

*   **Language:** C++17 (Native), QML (UI)
*   **Framework:** Qt 6.10 (Multimedia / Quick)
*   **Computer Vision:** OpenCV 4.10 Android SDK (NDK)
*   **Build System:** CMake
*   **Target:** Android ARM64-v8a (API 34+)

## 🚀 Build Instructions

### Prerequisites
1.  **Qt 6.10+** (Android ARM64 Kit installed).
2.  **Android NDK** (r26 or newer).
3.  **OpenCV Android SDK** (4.x).

### Environment Setup
To keep the build hermetic and machine-agnostic, this project uses an environment variable to locate OpenCV.

1.  Download the **OpenCV Android SDK**.
2.  Set the environment variable `OpenCV_Android` to the SDK root path.
    *   *Windows Example:* `C:\Libs\OpenCV-android-sdk`
    *   *Linux/Mac Example:* `~/libs/OpenCV-android-sdk`

### Compiling
1.  Open `CMakeLists.txt` in Qt Creator.
2.  Select the **Android Qt 6.10 Clang arm64-v8a** kit.
3.  Build and Deploy.

*Note: The first launch requires Runtime Camera Permissions. Grant them on the device manually.*

## 📝 License
MIT
