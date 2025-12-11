#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDirIterator>
#include <QDebug>
#include "FrameProcessor.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // --- INSPECT RESOURCES ---
    qDebug() << "--- Listing Embedded Resources ---";
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << it.next();
    }
    qDebug() << "--------------------------------";

    // Register C++ class so QML can see it
    qmlRegisterType<FrameProcessor>("com.systems.inspector", 1, 0, "FrameProcessor");

    QQmlApplicationEngine engine;
    // URI "com.mrgrey.systems" + File "Main.qml" becomes:
    const QUrl url(u"qrc:/com/systems/inspector/content/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
