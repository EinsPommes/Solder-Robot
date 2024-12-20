#include "energy_manager.h"
#include <QDebug>
#include <cmath>

EnergyManager::EnergyManager(QObject *parent)
    : QObject(parent)
    , monitoringTimer(new QTimer(this))
    , powerSavingEnabled(false)
{
    connect(monitoringTimer, &QTimer::timeout, 
            this, &EnergyManager::monitorPowerConsumption);
    monitoringTimer->start(1000); // Jede Sekunde aktualisieren
}

PowerConsumption EnergyManager::getCurrentConsumption() const {
    PowerConsumption consumption;
    consumption.timestamp = QDateTime::currentDateTime();
    
    // Hier würden die tatsächlichen Messwerte ausgelesen werden
    // Für dieses Beispiel simulieren wir die Werte
    consumption.heaterPower = 200.0 * (1.0 + 0.1 * std::sin(QDateTime::currentSecsSinceEpoch() / 10.0));
    consumption.motorsPower = 50.0 * (1.0 + 0.05 * std::cos(QDateTime::currentSecsSinceEpoch() / 5.0));
    consumption.electronicsPower = 30.0;
    consumption.totalPower = consumption.heaterPower + consumption.motorsPower + 
                           consumption.electronicsPower;
    
    return consumption;
}

double EnergyManager::getTotalEnergyUsage(const QDateTime &start, 
                                        const QDateTime &end) const {
    double totalEnergy = 0.0;
    
    // Energieverbrauch aus dem Verlauf berechnen
    for (const auto &consumption : consumptionHistory) {
        if (consumption.timestamp >= start && consumption.timestamp <= end) {
            // Energie in Wattstunden
            totalEnergy += consumption.totalPower / 3600.0;
        }
    }
    
    return totalEnergy;
}

void EnergyManager::setEnergyProfile(const EnergyProfile &profile) {
    currentProfile = profile;
    
    // Neue Limits anwenden
    checkPowerLimits();
    
    if (powerSavingEnabled) {
        applyPowerSavingMeasures();
    }
}

void EnergyManager::enablePowerSaving(bool enable) {
    powerSavingEnabled = enable;
    
    if (enable) {
        applyPowerSavingMeasures();
        emit enteringStandby();
    } else {
        // Normale Betriebsparameter wiederherstellen
        emit exitingStandby();
    }
}

void EnergyManager::scheduleStandby(const QTime &start, const QTime &end) {
    // Timer für automatischen Standby einrichten
    QTime currentTime = QTime::currentTime();
    
    if (currentTime >= start && currentTime < end) {
        enablePowerSaving(true);
    }
    
    // Timer für nächsten Tag planen
    int secsToStart = currentTime.secsTo(start);
    if (secsToStart < 0) {
        secsToStart += 24 * 3600;
    }
    
    QTimer::singleShot(secsToStart * 1000, this, [this]() {
        enablePowerSaving(true);
    });
}

void EnergyManager::optimizeHeatingCycle() {
    // Temperaturkurve analysieren und optimieren
    double optimalTemp = calculateOptimalTemperature();
    
    // Vorhersage der Energieeinsparung
    double currentEnergy = getTotalEnergyUsage(
        QDateTime::currentDateTime().addSecs(-3600),
        QDateTime::currentDateTime()
    );
    
    double predictedEnergy = currentEnergy * 0.85; // Geschätzte Einsparung
    double savings = currentEnergy - predictedEnergy;
    
    emit temperatureOptimized(optimalTemp);
    emit energySavingsCalculated(savings);
}

void EnergyManager::predictTemperatureCurve(double targetTemp) {
    // Aufheizkurve simulieren
    const int steps = 60; // 1 Minute in Sekunden
    double currentTemp = 25.0; // Raumtemperatur
    double heatingRate = (targetTemp - currentTemp) / steps;
    
    for (int i = 0; i < steps; ++i) {
        currentTemp += heatingRate;
        // Hier würde die Vorhersage in einer Datenstruktur gespeichert
    }
}

void EnergyManager::adjustPowerLimit(double limit) {
    currentProfile.maxPower = limit;
    checkPowerLimits();
}

QVector<PowerConsumption> EnergyManager::getConsumptionHistory(
    const QDateTime &start, const QDateTime &end) const {
    QVector<PowerConsumption> filteredHistory;
    
    for (const auto &consumption : consumptionHistory) {
        if (consumption.timestamp >= start && consumption.timestamp <= end) {
            filteredHistory.append(consumption);
        }
    }
    
    return filteredHistory;
}

double EnergyManager::getAverageConsumption() const {
    if (consumptionHistory.isEmpty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (const auto &consumption : consumptionHistory) {
        sum += consumption.totalPower;
    }
    
    return sum / consumptionHistory.size();
}

double EnergyManager::getPeakConsumption() const {
    double peak = 0.0;
    
    for (const auto &consumption : consumptionHistory) {
        peak = std::max(peak, consumption.totalPower);
    }
    
    return peak;
}

void EnergyManager::monitorPowerConsumption() {
    PowerConsumption current = getCurrentConsumption();
    
    // Verlauf aktualisieren
    consumptionHistory.append(current);
    
    // Verlauf auf maximal 24 Stunden begrenzen
    while (consumptionHistory.size() > 86400) {
        consumptionHistory.removeFirst();
    }
    
    emit powerConsumptionChanged(current);
    
    // Leistungsgrenzen überprüfen
    if (current.totalPower > currentProfile.maxPower) {
        emit powerLimitExceeded(current.totalPower, currentProfile.maxPower);
    }
}

void EnergyManager::checkPowerLimits() {
    PowerConsumption current = getCurrentConsumption();
    
    // Komponenten einzeln überprüfen
    for (const auto &limit : currentProfile.componentLimits) {
        if (current.heaterPower > limit.value()) {
            // Heizleistung reduzieren
        }
        if (current.motorsPower > limit.value()) {
            // Motorleistung begrenzen
        }
    }
}

void EnergyManager::updateConsumptionHistory() {
    // Alte Einträge entfernen
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-1);
    
    while (!consumptionHistory.isEmpty() && 
           consumptionHistory.first().timestamp < cutoff) {
        consumptionHistory.removeFirst();
    }
}

double EnergyManager::calculateOptimalTemperature() {
    // Temperaturoptimierung basierend auf Verbrauchsdaten
    double avgConsumption = getAverageConsumption();
    double peakConsumption = getPeakConsumption();
    
    // Optimale Temperatur berechnen
    double optimalTemp = 350.0; // Basis-Löttemperatur
    
    if (avgConsumption > 0.8 * peakConsumption) {
        // Temperatur reduzieren bei hoher Durchschnittsauslastung
        optimalTemp *= 0.95;
    }
    
    return optimalTemp;
}

void EnergyManager::applyPowerSavingMeasures() {
    if (!powerSavingEnabled) return;
    
    // Standby-Leistung anwenden
    currentProfile.maxPower = currentProfile.standbyPower;
    
    // Komponenten in Energiesparmodus versetzen
    for (auto &limit : currentProfile.componentLimits) {
        limit = limit * 0.5; // 50% Leistungsreduktion
    }
    
    predictEnergyUsage();
}

void EnergyManager::predictEnergyUsage() {
    // Energieverbrauch für die nächsten 24 Stunden vorhersagen
    double predictedEnergy = 0.0;
    double avgConsumption = getAverageConsumption();
    
    // Einfache lineare Vorhersage
    predictedEnergy = avgConsumption * 24.0;
    
    // Hier könnte eine komplexere Vorhersage implementiert werden
    qDebug() << "Vorhergesagter Energieverbrauch (24h):" << predictedEnergy << "Wh";
}
