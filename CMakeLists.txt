cmake_minimum_required(VERSION 3.16)

project(RivianCamTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS Quick Multimedia)

qt_standard_project_setup()

qt_add_executable(appRivianCamTest
    src/main.cpp
    src/FrameProcessor.h
)

qt_add_qml_module(appRivianCamTest
    URI "com.mrgrey.systems"
    VERSION 1.0
    QML_FILES content/Main.qml
    RESOURCES android/AndroidManifest.xml android/build.gradle android/res/values/libs.xml android/res/xml/qtprovider_paths.xml
    RESOURCES android/AndroidManifest.xml android/build.gradle android/res/values/libs.xml android/res/xml/qtprovider_paths.xml android/gradle/wrapper/gradle-wrapper.jar android/gradle/wrapper/gradle-wrapper.properties android/gradle.properties android/gradlew android/gradlew.bat
    RESOURCES android/AndroidManifest.xml android/build.gradle android/res/values/libs.xml android/res/xml/qtprovider_paths.xml android/gradle/wrapper/gradle-wrapper.jar android/gradle/wrapper/gradle-wrapper.properties android/gradle.properties android/gradlew android/gradlew.bat
)

target_link_libraries(appRivianCamTest
    PRIVATE Qt6::Quick Qt6::Multimedia
)

# Android specific settings
if(ANDROID)
    set_property(TARGET appRivianCamTest PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
endif()
