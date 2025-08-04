import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 900
    height: 700
    minimumWidth: 400
    minimumHeight: 500
    visible: true
    title: qsTr("Mesh Chat Pro")

    // Modern color scheme
    property color primaryColor: "#2196F3"
    property color primaryDark: "#1976D2"
    property color accentColor: "#03DAC6"
    property color backgroundColor: "#F5F5F5"
    property color cardColor: "#FFFFFF"
    property color textColor: "#212121"
    property color secondaryTextColor: "#757575"
    property color errorColor: "#F44336"
    property color successColor: "#4CAF50"

    color: backgroundColor

    // Header - Sadece gradient, color yok
    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80
        color: "transparent"

        gradient: Gradient {
            GradientStop { position: 0.0; color: primaryColor }
            GradientStop { position: 1.0; color: primaryDark }
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 20

            Rectangle {
                width: 40
                height: 40
                radius: 20
                antialiasing: true
                color: accentColor

                Text {
                    anchors.centerIn: parent
                    text: "🔗"
                    font.pixelSize: 20
                }
            }

            Column {
                Layout.fillWidth: true
                Text {
                    text: "Mesh Chat Pro"
                    color: "white"
                    font.pixelSize: 24
                    font.bold: true
                }
                Text {
                    text: bluetoothManager.status
                    color: "#E3F2FD"
                    font.pixelSize: 14
                }
            }

            Rectangle {
                width: 12
                height: 12
                radius: 6
                antialiasing: true
                color: bluetoothManager.connectedDevices.length > 0 ? successColor : errorColor

                SequentialAnimation on opacity {
                    running: bluetoothManager.status.includes("Scanning")
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: 500 }
                    NumberAnimation { to: 1.0; duration: 500 }
                }
            }
        }
    }

    ScrollView {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20

        Column {
            width: parent.width
            spacing: 20

            // Control Panel Card
            Rectangle {
                width: parent.width
                height: controlColumn.implicitHeight + 40
                color: cardColor
                radius: 12
                antialiasing: true

                // Basit gölge efekti
                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: 2
                    anchors.leftMargin: 2
                    radius: 12
                    antialiasing: true
                    color: "#E0E0E0"
                    z: -1
                }

                Column {
                    id: controlColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20

                    Text {
                        text: "🎛️ Control Panel"
                        font.pixelSize: 18
                        font.bold: true
                        color: textColor
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 12

                        // Modern Button - Sadece color, gradient yok
                        Button {
                            text: "🔍 Scan"
                            height: 48
                            Layout.fillWidth: true

                            background: Rectangle {
                                radius: 8
                                antialiasing: true
                                color: parent.pressed ? primaryDark : primaryColor
                            }

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 14
                                font.bold: true
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: bluetoothManager.startDiscovery()
                        }

                        Button {
                            id: startServerButton // ID eklemek iyi bir pratik olabilir
                            text: "🚀 Start Server"
                            height: 48
                            Layout.fillWidth: true

                            background: Rectangle {
                                radius: 8
                                antialiasing: true
                                // Butona basıldığında renk değiştirmesi için 'pressed' durumunu ekleyebilirsiniz
                                color: startServerButton.pressed ? primaryDark : "steelblue"
                            }

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 14
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: bluetoothManager.startServer()
                        }

                        Button {
                            text: "🔗 Auto Connect"
                            height: 48
                            Layout.fillWidth: true

                            background: Rectangle {
                                radius: 8
                                antialiasing: true
                                color: "#00796B"
                            }

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 14
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: bluetoothManager.autoConnectToMesh()
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: 60
                        radius: 8
                        antialiasing: true
                        color: "#F8F9FA"
                        border.color: "#E9ECEF"
                        border.width: 1

                        Row {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 30

                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    text: bluetoothManager.discoveredDevices.length.toString()
                                    font.pixelSize: 24
                                    font.bold: true
                                    color: primaryColor
                                }
                                Text {
                                    text: "Discovered"
                                    font.pixelSize: 12
                                    color: secondaryTextColor
                                }
                            }

                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    text: bluetoothManager.connectedDevices.length.toString()
                                    font.pixelSize: 24
                                    font.bold: true
                                    color: successColor
                                }
                                Text {
                                    text: "Connected"
                                    font.pixelSize: 12
                                    color: secondaryTextColor
                                }
                            }

                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                Text {
                                    text: bluetoothManager.messages.length.toString()
                                    font.pixelSize: 24
                                    font.bold: true
                                    color: accentColor
                                }
                                Text {
                                    text: "Messages"
                                    font.pixelSize: 12
                                    color: secondaryTextColor
                                }
                            }
                        }
                    }
                }
            }

            // Devices Card
            Rectangle {
                width: parent.width
                height: deviceColumn.implicitHeight + 40
                color: cardColor
                radius: 12
                antialiasing: true

                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: 2
                    anchors.leftMargin: 2
                    radius: 12
                    antialiasing: true
                    color: "#E0E0E0"
                    z: -1
                }

                Column {
                    id: deviceColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Text {
                        text: "📱 Devices"
                        font.pixelSize: 18
                        font.bold: true
                        color: textColor
                    }

                    Text {
                        text: "Available Devices"
                        font.pixelSize: 14
                        font.bold: true
                        color: secondaryTextColor
                    }

                    Rectangle {
                        width: parent.width
                        height: Math.max(120, discoveredList.contentHeight + 20)
                        radius: 8
                        antialiasing: true
                        color: "#FAFAFA"
                        border.color: "#E0E0E0"
                        border.width: 1

                        ListView {
                            id: discoveredList
                            anchors.fill: parent
                            anchors.margins: 10
                            model: bluetoothManager.discoveredDevices
                            spacing: 6

                            delegate: Rectangle {
                                width: discoveredList.width
                                height: 45
                                radius: 6
                                antialiasing: true
                                color: deviceMouseArea.pressed ? primaryColor : cardColor
                                border.color: "#E0E0E0"
                                border.width: 1

                                Row {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 12

                                    Rectangle {
                                        width: 28
                                        height: 28
                                        radius: 14
                                        antialiasing: true
                                        color: primaryColor
                                        anchors.verticalCenter: parent.verticalCenter

                                        Text {
                                            anchors.centerIn: parent
                                            text: "📱"
                                            font.pixelSize: 14
                                            color: "white"
                                        }
                                    }

                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: modelData
                                        font.pixelSize: 13
                                        color: deviceMouseArea.pressed ? "white" : textColor
                                        elide: Text.ElideRight
                                        width: parent.width - 50
                                    }
                                }

                                MouseArea {
                                    id: deviceMouseArea
                                    anchors.fill: parent
                                    onClicked: {
                                        var address = modelData.split("(")[1].split(")")[0]
                                        bluetoothManager.connectToDevice(address)
                                    }
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "No devices found\nTap 'Scan' to discover devices"
                            color: secondaryTextColor
                            font.pixelSize: 14
                            horizontalAlignment: Text.AlignHCenter
                            visible: bluetoothManager.discoveredDevices.length === 0
                        }
                    }

                    Text {
                        text: "Connected Devices"
                        font.pixelSize: 14
                        font.bold: true
                        color: secondaryTextColor
                    }

                    Rectangle {
                        width: parent.width
                        height: Math.max(80, connectedList.contentHeight + 20)
                        radius: 8
                        antialiasing: true
                        color: "#FAFAFA"
                        border.color: "#E0E0E0"
                        border.width: 1

                        ListView {
                            id: connectedList
                            anchors.fill: parent
                            anchors.margins: 10
                            model: bluetoothManager.connectedDevices
                            spacing: 6

                            delegate: Rectangle {
                                width: connectedList.width
                                height: 35
                                radius: 6
                                antialiasing: true
                                color: cardColor
                                border.color: successColor
                                border.width: 1

                                Row {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 10

                                    Rectangle {
                                        width: 20
                                        height: 20
                                        radius: 10
                                        antialiasing: true
                                        color: successColor
                                        anchors.verticalCenter: parent.verticalCenter

                                        Text {
                                            anchors.centerIn: parent
                                            text: "✓"
                                            font.pixelSize: 10
                                            color: "white"
                                        }
                                    }

                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: modelData
                                        font.pixelSize: 13
                                        color: textColor
                                    }
                                }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "No connected devices"
                            color: secondaryTextColor
                            font.pixelSize: 14
                            visible: bluetoothManager.connectedDevices.length === 0
                        }
                    }
                }
            }

            // Chat Card
            Rectangle {
                width: parent.width
                height: 400
                color: cardColor
                radius: 12
                antialiasing: true

                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: 2
                    anchors.leftMargin: 2
                    radius: 12
                    antialiasing: true
                    color: "#E0E0E0"
                    z: -1
                }

                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Text {
                        text: "💬 Messages"
                        font.pixelSize: 18
                        font.bold: true
                        color: textColor
                    }

                    Rectangle {
                        width: parent.width
                        height: 250
                        radius: 8
                        antialiasing: true
                        color: "#FAFAFA"
                        border.color: "#E0E0E0"
                        border.width: 1

                        ListView {
                            id: messagesList
                            anchors.fill: parent
                            anchors.margins: 10
                            model: bluetoothManager.messages
                            spacing: 6
                            clip: true

                            delegate: Rectangle {
                                width: messagesList.width
                                height: messageText.implicitHeight + 16
                                radius: 6
                                antialiasing: true
                                color: modelData.includes("Sent:") ? primaryColor : cardColor
                                border.color: "#E0E0E0"
                                border.width: modelData.includes("Sent:") ? 0 : 1

                                Text {
                                    id: messageText
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    text: modelData
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 13
                                    color: modelData.includes("Sent:") ? "white" : textColor
                                }
                            }

                            onCountChanged: positionViewAtEnd()
                        }

                        Text {
                            anchors.centerIn: parent
                            text: "No messages yet\nSend a message to start chatting!"
                            color: secondaryTextColor
                            font.pixelSize: 14
                            horizontalAlignment: Text.AlignHCenter
                            visible: bluetoothManager.messages.length === 0
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 10

                        Rectangle { // Border
                            width: parent.width - 120
                            height: 40
                            radius: 20
                            antialiasing: true
                            color: messageInput.focus ? primaryColor : "#E0E0E0"

                            Rectangle { // Fill
                                anchors.fill: parent
                                anchors.margins: 2 // This is the border width
                                radius: parent.radius - anchors.margins
                                antialiasing: true
                                color: "#FAFAFA"

                                TextField {
                                    id: messageInput
                                    anchors.fill: parent
                                    anchors.margins: 10 // Original 12 - 2 for border
                                    placeholderText: "Type your message..."
                                    font.pixelSize: 14
                                    color: textColor
                                    background: null

                                    Keys.onReturnPressed: sendButton.clicked()
                                }
                            }
                        }

                        Button {
                            id: sendButton
                            text: "Send"
                            height: 40
                            width: 60
                            enabled: messageInput.text.trim() !== ""

                            background: Rectangle {
                                radius: 6
                                antialiasing: true
                                color: parent.enabled ? (parent.pressed ? primaryDark : primaryColor) : "#E0E0E0"
                            }

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 12
                                font.bold: true
                                color: parent.enabled ? "white" : "#9E9E9E"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                if (messageInput.text.trim() !== "") {
                                    bluetoothManager.sendMessage(messageInput.text.trim())
                                    messageInput.text = ""
                                }
                            }
                        }

                        Button {
                            text: "📡"
                            height: 40
                            width: 40
                            enabled: messageInput.text.trim() !== ""

                            background: Rectangle {
                                radius: 6
                                antialiasing: true
                                color: parent.enabled ? (parent.pressed ? "#00796B" : accentColor) : "#E0E0E0"
                            }

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 16
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                if (messageInput.text.trim() !== "") {
                                    bluetoothManager.broadcastMessage(messageInput.text.trim())
                                    messageInput.text = ""
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
