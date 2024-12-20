#ifndef SOLDERROBOT_MAINTENANCE_SYSTEM_H
#define SOLDERROBOT_MAINTENANCE_SYSTEM_H

#include <QObject>
#include <QDateTime>
#include <QMap>

struct MaintenanceTask {
    QString name;
    QString description;
    int intervalHours;
    QDateTime lastPerformed;
    bool isRequired;
    int priority;  // 1-5, 5 highest
};

struct ComponentStatus {
    QString name;
    double wearLevel;       // 0.0 - 1.0
    int operatingHours;
    QDateTime installDate;
    QDateTime nextMaintenance;
    bool needsAttention;
};

class MaintenanceSystem : public QObject {
    Q_OBJECT

public:
    explicit MaintenanceSystem(QObject *parent = nullptr);
    
    // Wartungsplanung
    void addMaintenanceTask(const MaintenanceTask &task);
    void updateTaskStatus(const QString &taskName, const QDateTime &performed);
    QVector<MaintenanceTask> getDueTasks() const;
    
    // Komponentenüberwachung
    void updateComponentStatus(const QString &component, double wearLevel);
    ComponentStatus getComponentStatus(const QString &component) const;
    QVector<ComponentStatus> getAllComponentStatus() const;
    
    // Predictive Maintenance
    double predictFailureProbability(const QString &component) const;
    QDateTime estimateNextMaintenance(const QString &component) const;
    
    // Wartungsprotokoll
    void logMaintenanceEvent(const QString &component, const QString &action);
    QVector<QString> getMaintenanceHistory(const QString &component) const;
    
signals:
    void maintenanceRequired(const QString &component, const QString &reason);
    void componentWearCritical(const QString &component, double wearLevel);
    void maintenanceCompleted(const QString &component);
    void predictiveWarning(const QString &component, double probability);

private:
    QMap<QString, MaintenanceTask> tasks;
    QMap<QString, ComponentStatus> components;
    QMap<QString, QVector<QString>> maintenanceLog;
    
    void checkMaintenanceSchedule();
    void analyzeWearPatterns();
    double calculateWearRate(const QString &component) const;
    void updatePredictions();
};

#endif // SOLDERROBOT_MAINTENANCE_SYSTEM_H
