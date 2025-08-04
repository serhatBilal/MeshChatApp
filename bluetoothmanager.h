#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QBluetoothServiceInfo>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothServer>
#include <QTime>
#include <QTimer>
#include <QMap>
#include <QBluetoothAddress>
#include <QStringList>

class BluetoothManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QStringList discoveredDevices READ discoveredDevices NOTIFY discoveredDevicesChanged)
    Q_PROPERTY(QStringList messages READ messages NOTIFY messagesChanged)
    Q_PROPERTY(QStringList connectedDevices READ connectedDevices NOTIFY connectedDevicesChanged)

public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager();

    QString status() const;
    QStringList discoveredDevices() const;
    QStringList messages() const;
    QStringList connectedDevices() const;

public slots:
    void startDiscovery();
    void sendMessage(const QString &message);
    void connectToDevice(const QString &address);
    void startServer();
    void broadcastMessage(const QString &message);

signals:
    void statusChanged();
    void discoveredDevicesChanged();
    void messagesChanged();
    void connectedDevicesChanged();

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void deviceDiscoveryFinished();
    void deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error);

    void socketConnected();
    void socketDisconnected();
    void socketReadyRead();
    void socketErrorOccurred(QBluetoothSocket::SocketError error);

    void serverNewConnection();
    void clientDisconnected();

    void autoConnectToMesh();

private:
    void setStatus(const QString &status);
    void addMessage(const QString &message);
    void addConnectedDevice(const QString &address);
    void removeConnectedDevice(const QString &address);
    void meshBroadcast(const QString &message, const QString &senderAddress = QString());
    void connectToMeshDevice(const QBluetoothDeviceInfo &device);
    bool isMeshMessage(const QString &message) const;
    QString extractOriginalMessage(const QString &meshMessage) const;

    QBluetoothLocalDevice *m_localDevice;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QBluetoothServer *m_server;
    QMap<QString, QBluetoothSocket*> m_sockets; // Address -> Socket mapping

    QString m_status;
    QStringList m_discoveredDevices;
    QStringList m_messages;
    QStringList m_connectedDevices;
    QStringList m_processedMessages; // Flood control için

    quint16 m_servicePort;
    QTimer *m_autoConnectTimer; // Otomatik bağlantı için
};

#endif // BLUETOOTHMANAGER_H
