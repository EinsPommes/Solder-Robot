#ifndef SOLDERROBOT_ENERGY_MANAGER_H
#define SOLDERROBOT_ENERGY_MANAGER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>

struct PowerConsumption {
    double totalPower;      // Watt
    double heaterPower;     // Watt
    double motorsPower;     // Watt
    double electronicsPower;// Watt
    QDateTime timestamp;
};

struct EnergyProfile {
    QString name;
    double maxPower;
    double standbyPower;
    int warmupTime;         // Sekunden
    int cooldownTime;       // Sekunden
    QMap<QString, double> componentLimits;
};

class EnergyManager : public QObject {
    Q_OBJECT

public:
    explicit EnergyManager(QObject *parent = nullptr);
    
    // Energieüberwachung
    PowerConsumption getCurrentConsumption() const;
    double getTotalEnergyUsage(const QDateTime &start, const QDateTime &end) const;
    
    // Energieprofile
    void setEnergyProfile(const EnergyProfile &profile);
    void enablePowerSaving(bool enable);
    void scheduleStandby(const QTime &start, const QTime &end);
    
    // Temperaturoptimierung
    void optimizeHeatingCycle();
    void predictTemperatureCurve(double targetTemp);
    void adjustPowerLimit(double limit);
    
    // Statistiken
    QVector<PowerConsumption> getConsumptionHistory(const QDateTime &start,
                                                   const QDateTime &end) const;
    double getAverageConsumption() const;
    double getPeakConsumption() const;
    
signals:
    void powerConsumptionChanged(const PowerConsumption &consumption);
    void powerLimitExceeded(double current, double limit);
    void enteringStandby();
    void exitingStandby();
    void temperatureOptimized(double newTarget);
    void energySavingsCalculated(double savedPower);

private:
    QTimer *monitoringTimer;
    EnergyProfile currentProfile;
    bool powerSavingEnabled;
    QVector<PowerConsumption> consumptionHistory;
    
    void monitorPowerConsumption();
    void checkPowerLimits();
    void updateConsumptionHistory();
    double calculateOptimalTemperature();
    void applyPowerSavingMeasures();
    void predictEnergyUsage();
};

#endif // SOLDERROBOT_ENERGY_MANAGER_H
