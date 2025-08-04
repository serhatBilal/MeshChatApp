#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QPermissions>
#include <QBluetoothPermission>
#include <QDebug>
#include "bluetoothmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QBluetoothPermission bluetoothPermission;

    switch (qApp->checkPermission(bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(bluetoothPermission, [](const QPermission &permission) {
            if (permission.status() == Qt::PermissionStatus::Granted) {
                qDebug() << "Bluetooth permission granted";
            } else {
                qDebug() << "Bluetooth permission denied";
            }
        });
        break;
    case Qt::PermissionStatus::Denied:
        qDebug() << "Bluetooth permission denied";
        break;
    case Qt::PermissionStatus::Granted:
        qDebug() << "Bluetooth permission already granted";
        break;
    }

    BluetoothManager bluetoothManager;

    bluetoothManager.startServer();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("bluetoothManager", &bluetoothManager);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("MeshChatApp", "Main");

    return app.exec();
}
