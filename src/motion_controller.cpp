#include "motion_controller.h"
#include <QDebug>

MotionController::MotionController(QObject *parent)
    : QObject(parent)
    , serialPort(new QSerialPort(this))
    , currentX(0)
    , currentY(0)
    , currentZ(0)
    , currentConveyorSpeed(0)
    , isInitialized(false)
{
}

MotionController::~MotionController() {
    if (serialPort->isOpen()) {
        serialPort->close();
    }
}

bool MotionController::initialize() {
    if (isInitialized) return true;
    
    if (!connectToHardware()) {
        emit errorOccurred("Verbindung zur Hardware konnte nicht hergestellt werden");
        return false;
    }
    
    // Initialisierungs-G-Code senden
    sendGCode("G21"); // Metrische Einheiten
    sendGCode("G90"); // Absolute Positionierung
    sendGCode("G28"); // Referenzfahrt aller Achsen
    
    isInitialized = true;
    return true;
}

bool MotionController::connectToHardware() {
    // COM-Port-Einstellungen
    serialPort->setPortName("COM3"); // Anpassen an tatsächlichen Port
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    if (!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Fehler beim Öffnen des seriellen Ports:" << serialPort->errorString();
        return false;
    }
    
    return true;
}

void MotionController::moveToPosition(double x, double y, double z) {
    if (!isInitialized) return;
    
    // Bewegungsbefehl generieren
    QString command = QString("G1 X%1 Y%2 Z%3 F3000")
        .arg(x, 0, 'f', 3)
        .arg(y, 0, 'f', 3)
        .arg(z, 0, 'f', 3);
    
    sendGCode(command);
    
    // Aktuelle Position aktualisieren
    currentX = x;
    currentY = y;
    currentZ = z;
    
    emit positionChanged(currentX, currentY, currentZ);
}

void MotionController::setConveyorSpeed(int speed) {
    if (!isInitialized) return;
    
    // Geschwindigkeit in M-Code umwandeln (z.B. M106 für Lüfter/Motor)
    QString command = QString("M106 S%1").arg(speed * 255 / 100);
    sendGCode(command);
    
    currentConveyorSpeed = speed;
    emit conveyorSpeedChanged(speed);
}

void MotionController::emergencyStop() {
    if (!isInitialized) return;
    
    // Sofort-Stopp-Befehl senden
    sendGCode("M112"); // Emergency Stop
    
    // Alle Motoren deaktivieren
    sendGCode("M18");
    
    isInitialized = false;
}

void MotionController::sendGCode(const QString &command) {
    if (!serialPort->isOpen()) return;
    
    // Befehl mit Zeilenumbruch senden
    QByteArray data = (command + "\n").toUtf8();
    serialPort->write(data);
    serialPort->flush();
    
    qDebug() << "G-Code gesendet:" << command;
    
    // Auf Antwort warten
    if (!serialPort->waitForBytesWritten(1000)) {
        emit errorOccurred("Timeout beim Senden des G-Code");
        return;
    }
    
    // Antwort lesen (optional)
    if (serialPort->waitForReadyRead(1000)) {
        QByteArray responseData = serialPort->readAll();
        qDebug() << "Antwort erhalten:" << responseData;
    }
}
