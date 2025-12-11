cmake_minimum_required(VERSION 3.16)

project(AndroidHALInspector VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS Quick Multimedia)


# --- OpenCV Configuration (Environment Variable) ---
# We use a specific variable 'OpenCV_Android' to avoid conflict with Desktop OpenCV
set(CV_ROOT $ENV{OpenCV_Android})

if(NOT EXISTS "${CV_ROOT}")
    message(FATAL_ERROR "
    Error: Environment variable 'OpenCV_Android' is not set or invalid.
    Please set 'OpenCV_Android' to the root of the extracted OpenCV SDK.
    Current Value: '${CV_ROOT}'
    ")
endif()

# Point CMake to the Android JNI configuration scripts inside the SDK
set(OpenCV_DIR "${CV_ROOT}/sdk/native/jni")

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV Android Configuration:")
message(STATUS "   Root: ${CV_ROOT}")
message(STATUS "   Version: ${OpenCV_VERSION}")
message(STATUS "   Libs: ${OpenCV_LIBS}")

qt_standard_project_setup()

qt_add_executable(AndroidHALInspector
    src/main.cpp
    src/FrameProcessor.h
    src/FrameProcessor.cpp
)

# QML Module Definition
qt_add_qml_module(AndroidHALInspector
    URI "com.systems.inspector"
    VERSION 1.0
    QML_FILES content/Main.qml
)

# Linking
target_link_libraries(AndroidHALInspector
    PRIVATE Qt6::Quick Qt6::Multimedia ${OpenCV_LIBS}
)

if(OpenCV_FOUND)
    target_compile_definitions(AndroidHALInspector PRIVATE USE_OPENCV)
    target_link_libraries(AndroidHALInspector PRIVATE ${OpenCV_LIBS})
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
else()
    message(WARNING "OpenCV not found. Building without CV analysis features.")
endif()

# Android Manifest Integration
if(ANDROID)
    set_property(TARGET AndroidHALInspector PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
endif()
