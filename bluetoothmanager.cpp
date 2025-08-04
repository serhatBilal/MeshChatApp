#include "bluetoothmanager.h"
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QBluetoothServiceInfo>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
    , m_localDevice(new QBluetoothLocalDevice(this))
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
    , m_server(new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this))
    , m_servicePort(0)
    , m_autoConnectTimer(new QTimer(this))
{
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothManager::deviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothManager::deviceDiscoveryFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, &BluetoothManager::deviceDiscoveryError);

    // Server bağlantıları için
    connect(m_server, &QBluetoothServer::newConnection,
            this, &BluetoothManager::serverNewConnection);

    // Otomatik bağlantı timer'ı
    connect(m_autoConnectTimer, &QTimer::timeout, this, &BluetoothManager::autoConnectToMesh);
    m_autoConnectTimer->setInterval(30000); // 30 saniyede bir
    m_autoConnectTimer->start();

    setStatus("Ready");
}

BluetoothManager::~BluetoothManager()
{
}

QString BluetoothManager::status() const
{
    return m_status;
}

QStringList BluetoothManager::discoveredDevices() const
{
    return m_discoveredDevices;
}

QStringList BluetoothManager::messages() const
{
    return m_messages;
}

QStringList BluetoothManager::connectedDevices() const
{
    return m_connectedDevices;
}

void BluetoothManager::startDiscovery()
{
    if (m_discoveryAgent->isActive()) {
        m_discoveryAgent->stop();
    }

    m_discoveredDevices.clear();
    emit discoveredDevicesChanged();
    setStatus("Scanning...");
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
}

void BluetoothManager::sendMessage(const QString &message)
{
    // Tüm bağlı cihazlara mesaj gönder
    for (auto it = m_sockets.begin(); it != m_sockets.end(); ++it) {
        QBluetoothSocket *socket = it.value();
        if (socket && socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
            socket->write(message.toUtf8() + "\n");
        }
    }
    addMessage("Sent: " + message);
}

void BluetoothManager::connectToDevice(const QString &address)
{
    // Eğer zaten bu adrese bağlantı varsa, tekrar bağlanma
    if (m_sockets.contains(address)) {
        QBluetoothSocket *socket = m_sockets[address];
        if (socket && socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
            setStatus("Already connected to " + address);
            return;
        }
    }

    setStatus("Connecting to " + address + "...");
    QBluetoothAddress btAddress(address);

    QBluetoothSocket *socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    m_sockets[address] = socket;

    connect(socket, &QBluetoothSocket::connected, this, &BluetoothManager::socketConnected);
    connect(socket, &QBluetoothSocket::disconnected, this, &BluetoothManager::socketDisconnected);
    connect(socket, &QBluetoothSocket::readyRead, this, &BluetoothManager::socketReadyRead);
    connect(socket, &QBluetoothSocket::errorOccurred, this, &BluetoothManager::socketErrorOccurred);

    socket->connectToService(btAddress, QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort), QIODevice::ReadWrite);
}

void BluetoothManager::startServer()
{
    if (m_server->isListening()) {
        setStatus("Server already running");
        return;
    }

    // RFCOMM servisini başlat
    QBluetoothUuid serviceUuid(QBluetoothUuid::ServiceClassUuid::SerialPort);

    if (m_server->listen(QBluetoothAddress(), 1)) { // Port 1 kullan
        m_servicePort = m_server->serverPort();

        // Service info oluştur ve kaydet
        QBluetoothServiceInfo serviceInfo;
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, "MeshChat Service");
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription, "Mesh Chat Service for Bluetooth Mesh Network");
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, "MeshChatApp");
        serviceInfo.setServiceUuid(serviceUuid);

        QBluetoothServiceInfo::Sequence classId;
        classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
        serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

        // Protocol descriptor list
        QBluetoothServiceInfo::Sequence protocolDescriptorList;
        QBluetoothServiceInfo::Sequence protocol;
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
        protocolDescriptorList.append(QVariant::fromValue(protocol));
        protocol.clear();
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
                 << QVariant::fromValue(quint8(m_server->serverPort()));
        protocolDescriptorList.append(QVariant::fromValue(protocol));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

        // Service'i kaydet
        if (serviceInfo.registerService()) {
            setStatus("Server started on port " + QString::number(m_servicePort));
            addMessage("Server started");
        } else {
            setStatus("Failed to register service");
        }
    } else {
        setStatus("Failed to start server");
    }
}

void BluetoothManager::broadcastMessage(const QString &message)
{
    // Mesh ağda mesajı tüm bağlı cihazlara yayınla
    meshBroadcast(message);
}

void BluetoothManager::meshBroadcast(const QString &message, const QString &senderAddress)
{
    // Flood kontrolü: Aynı mesajı tekrar gönderme
    if (m_processedMessages.contains(message)) {
        return;
    }

    // Mesajı işlemiş olarak işaretle
    m_processedMessages.append(message);
    // Eski mesajları temizle (basit bir şekilde)
    if (m_processedMessages.size() > 100) {
        m_processedMessages.removeFirst();
    }

    // Mesh mesajı formatı
    QString meshMessage = "[MESH] " + message;

    // Mesajı gönderen cihaz hariç tüm bağlı cihazlara gönder
    for (auto it = m_sockets.begin(); it != m_sockets.end(); ++it) {
        QString address = it.key();
        QBluetoothSocket *socket = it.value();

        // Mesajı gönderen cihaza geri gönderme
        if (!senderAddress.isEmpty() && address == senderAddress) {
            continue;
        }

        if (socket && socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
            socket->write(meshMessage.toUtf8() + "\n");
        }
    }

    addMessage("Mesh Broadcast: " + message);
}

void BluetoothManager::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    QString deviceString = device.name() + " (" + device.address().toString() + ")";
    if (!m_discoveredDevices.contains(deviceString)) {
        m_discoveredDevices.append(deviceString);
        emit discoveredDevicesChanged();
    }
}

void BluetoothManager::deviceDiscoveryFinished()
{
    setStatus("Scan finished");
}

void BluetoothManager::deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    QString errorString;
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::NoError:
        errorString = "No error";
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        errorString = "Input/Output error";
        break;
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        errorString = "Bluetooth is powered off";
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        errorString = "Invalid Bluetooth adapter";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
        errorString = "Unsupported platform";
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
        errorString = "Unsupported discovery method";
        break;
    default:
        errorString = "Unknown error";
        break;
    }
    setStatus("Discovery error: " + errorString + " - " + m_discoveryAgent->errorString());
}

void BluetoothManager::socketConnected()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket*>(sender());
    if (socket) {
        QString address = socket->peerAddress().toString();
        addConnectedDevice(address);
        setStatus("Connected to " + address);
        addMessage("Connected to " + address);
    }
}

void BluetoothManager::socketDisconnected()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket*>(sender());
    if (socket) {
        QString address = socket->peerAddress().toString();
        removeConnectedDevice(address);
        m_sockets.remove(address);
        socket->deleteLater();
        setStatus("Disconnected from " + address);
        addMessage("Disconnected from " + address);
    }
}

void BluetoothManager::socketReadyRead()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket*>(sender());
    if (socket) {
        QString address = socket->peerAddress().toString();
        while (socket->canReadLine()) {
            QByteArray data = socket->readLine();
            if (!data.isEmpty()) {
                QString message = QString::fromUtf8(data);
                message = message.trimmed(); // Remove newline characters

                // Mesh mesajı kontrolü
                if (isMeshMessage(message)) {
                    // Orijinal mesajı çıkar
                    QString originalMessage = extractOriginalMessage(message);
                    addMessage("[MESH] " + address + ": " + originalMessage);

                    // Mesajı diğer cihazlara yönlendir
                    meshBroadcast(originalMessage, address);
                } else {
                    // Normal mesaj
                    addMessage(address + ": " + message);
                }
            }
        }
    }
}

bool BluetoothManager::isMeshMessage(const QString &message) const
{
    return message.startsWith("[MESH] ");
}

QString BluetoothManager::extractOriginalMessage(const QString &meshMessage) const
{
    if (isMeshMessage(meshMessage)) {
        return meshMessage.mid(7); // "[MESH] " uzunluğu 7
    }
    return meshMessage;
}

void BluetoothManager::socketErrorOccurred(QBluetoothSocket::SocketError error)
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket*>(sender());
    if (socket) {
        QString address = socket->peerAddress().toString();
        setStatus("Socket error with " + address + ": " + socket->errorString());
        removeConnectedDevice(address);
        m_sockets.remove(address);
        socket->deleteLater();
    }
}

void BluetoothManager::serverNewConnection()
{
    QBluetoothSocket *socket = m_server->nextPendingConnection();
    if (socket) {
        QString address = socket->peerAddress().toString();

        // Eğer bu adresten zaten bir bağlantı varsa, yeni bağlantıyı reddet
        if (m_sockets.contains(address)) {
            socket->disconnectFromService();
            socket->deleteLater();
            return;
        }

        m_sockets[address] = socket;

        connect(socket, &QBluetoothSocket::disconnected, this, &BluetoothManager::clientDisconnected);
        connect(socket, &QBluetoothSocket::readyRead, this, &BluetoothManager::socketReadyRead);
        connect(socket, &QBluetoothSocket::errorOccurred, this, &BluetoothManager::socketErrorOccurred);

        addConnectedDevice(address);
        setStatus("New connection from " + address);
        addMessage("New connection from " + address);
    }
}

void BluetoothManager::clientDisconnected()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket*>(sender());
    if (socket) {
        QString address = socket->peerAddress().toString();
        removeConnectedDevice(address);
        m_sockets.remove(address);
        socket->deleteLater();
        setStatus("Client disconnected: " + address);
        addMessage("Client disconnected: " + address);
    }
}

void BluetoothManager::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void BluetoothManager::addMessage(const QString &message)
{
    m_messages.append(QTime::currentTime().toString() + " - " + message);
    emit messagesChanged();
}

void BluetoothManager::addConnectedDevice(const QString &address)
{
    if (!m_connectedDevices.contains(address)) {
        m_connectedDevices.append(address);
        emit connectedDevicesChanged();
    }
}

void BluetoothManager::removeConnectedDevice(const QString &address)
{
    if (m_connectedDevices.contains(address)) {
        m_connectedDevices.removeAll(address);
        emit connectedDevicesChanged();
    }
}

void BluetoothManager::autoConnectToMesh()
{
    // Otomatik olarak mesh ağa bağlan
    if (m_discoveryAgent->isActive()) {
        return; // Zaten tarama yapılıyorsa bekle
    }

    // Mesh cihazlarına bağlan
    for (const QString &deviceString : m_discoveredDevices) {
        // Cihaz string'inden adresi çıkar
        QString address = deviceString.split("(")[1];
        address = address.split(")")[0];

        // Eğer zaten bağlıysak bağlanma
        if (m_connectedDevices.contains(address)) {
            continue;
        }

        // Cihaza bağlan
        connectToDevice(address);
    }
}

void BluetoothManager::connectToMeshDevice(const QBluetoothDeviceInfo &device)
{
    // Mesh cihazına bağlan
    QString address = device.address().toString();

    // Eğer zaten bağlıysak bağlanma
    if (m_connectedDevices.contains(address)) {
        return;
    }

    // Cihaza bağlan
    connectToDevice(address);
}
