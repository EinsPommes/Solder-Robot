#include "temperature_control.h"
#include <QDebug>
#include <cmath>

PIDController::PIDController(double kp, double ki, double kd)
    : kp(kp)
    , ki(ki)
    , kd(kd)
    , lastError(0.0)
    , integral(0.0)
{
}

double PIDController::calculate(double setpoint, double processVariable) {
    double error = setpoint - processVariable;
    integral += error;
    double derivative = error - lastError;
    lastError = error;
    
    return kp * error + ki * integral + kd * derivative;
}

TemperatureControl::TemperatureControl(QObject *parent)
    : QObject(parent)
    , updateTimer(new QTimer(this))
    , pidController(new PIDController(2.0, 0.5, 1.0))
    , targetTemperature(0.0)
    , currentTemperature(0.0)
    , heatingEnabled(false)
{
    connect(updateTimer, &QTimer::timeout, this, &TemperatureControl::updateTemperature);
}

TemperatureControl::~TemperatureControl() {
    updateTimer->stop();
    delete pidController;
}

bool TemperatureControl::initialize() {
    // Hier würde die tatsächliche Hardware-Initialisierung stattfinden
    
    updateTimer->start(100); // Alle 100ms aktualisieren
    return true;
}

void TemperatureControl::setTargetTemperature(double temperature) {
    targetTemperature = std::clamp(temperature, 0.0, 450.0);
    qDebug() << "Zieltemperatur gesetzt auf:" << targetTemperature;
}

double TemperatureControl::getCurrentTemperature() const {
    return currentTemperature;
}

void TemperatureControl::enableHeating(bool enable) {
    heatingEnabled = enable;
    if (!enable) {
        applyHeatingPower(0.0);
    }
}

void TemperatureControl::updateTemperature() {
    // Aktuelle Temperatur lesen
    double measuredTemp = readTemperatureSensor();
    
    if (heatingEnabled) {
        // PID-Regelung berechnen
        double power = pidController->calculate(targetTemperature, measuredTemp);
        power = std::clamp(power, 0.0, 100.0);
        
        // Heizleistung anwenden
        applyHeatingPower(power);
    }
    
    // Temperatur aktualisieren und Signal senden
    currentTemperature = measuredTemp;
    emit temperatureChanged(currentTemperature);
}

void TemperatureControl::applyHeatingPower(double power) {
    // Hier würde die tatsächliche Heizungssteuerung stattfinden
    // Für dieses Beispiel nur Logging
    qDebug() << "Heizleistung:" << power << "%";
}

double TemperatureControl::readTemperatureSensor() {
    // Hier würde der tatsächliche Temperatursensor ausgelesen werden
    // Für dieses Beispiel simulieren wir eine Temperaturänderung
    
    static const double heatingRate = 0.1;  // °C pro 100ms bei voller Leistung
    static const double coolingRate = 0.05; // °C pro 100ms
    
    if (heatingEnabled) {
        currentTemperature += (targetTemperature - currentTemperature) * heatingRate;
    } else {
        currentTemperature = std::max(25.0, currentTemperature - coolingRate);
    }
    
    return currentTemperature;
}
