import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    // GÖRÜNÜR BİR DİKDÖRTGEN EKLEYELİM
    Rectangle {
        anchors.fill: parent // Bu, dikdörtgenin pencereyi tamamen doldurmasını sağlar
        color: "steelblue"     // Dikdörtgenin rengini belirleyelim (istediğiniz rengi yazabilirsiniz)

        // İÇİNE BİR DE METİN KOYALIM
        Text {
            text: "Uygulamam Çalışıyor!"
            anchors.centerIn: parent // Metni dikdörtgenin ortasına yerleştirir
            color: "black"           // Metin rengi
            font.pixelSize: 24       // Yazı tipi boyutu
        }
    }
}
