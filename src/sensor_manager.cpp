#include "sensor_manager.h"
#include <QDebug>
#include <random> // Nur für Simulationszwecke

SensorManager::SensorManager(QObject *parent)
    : QObject(parent)
    , updateTimer(new QTimer(this))
    , lidarDistance(0.0)
    , magneticFieldStrength(0.0)
    , obstaclePresent(false)
    , componentPresent(false)
{
    connect(updateTimer, &QTimer::timeout, this, &SensorManager::updateSensorReadings);
}

SensorManager::~SensorManager() {
    updateTimer->stop();
}

bool SensorManager::initialize() {
    // Hier würde die tatsächliche Sensor-Initialisierung stattfinden
    // Für dieses Beispiel simulieren wir die Sensoren
    
    updateTimer->start(100); // Alle 100ms aktualisieren
    return true;
}

bool SensorManager::isObstacleDetected() const {
    return obstaclePresent;
}

bool SensorManager::isComponentPresent() const {
    return componentPresent;
}

double SensorManager::getLidarDistance() const {
    return lidarDistance;
}

double SensorManager::getMagneticFieldStrength() const {
    return magneticFieldStrength;
}

void SensorManager::updateSensorReadings() {
    // LiDAR-Sensor auslesen
    if (readLidarSensor()) {
        // Wenn Objekt zu nahe, Hindernis melden
        obstaclePresent = (lidarDistance < 50.0); // 50mm Sicherheitsabstand
        emit obstacleDetected(obstaclePresent);
    }
    
    // Magnetsensor auslesen
    if (readMagneticSensor()) {
        // Wenn Magnetfeld stark genug, Bauteil erkannt
        componentPresent = (magneticFieldStrength > 0.5);
        emit componentDetected(componentPresent);
    }
}

bool SensorManager::readLidarSensor() {
    // Hier würde der tatsächliche LiDAR-Sensor ausgelesen werden
    // Für dieses Beispiel simulieren wir Messwerte
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(40.0, 200.0);
    
    lidarDistance = dis(gen);
    return true;
}

bool SensorManager::readMagneticSensor() {
    // Hier würde der tatsächliche Magnetsensor ausgelesen werden
    // Für dieses Beispiel simulieren wir Messwerte
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    
    magneticFieldStrength = dis(gen);
    return true;
}
