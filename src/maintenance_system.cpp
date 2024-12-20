#include "maintenance_system.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

MaintenanceSystem::MaintenanceSystem(QObject *parent)
    : QObject(parent)
{
    // Standard-Wartungsaufgaben hinzufügen
    MaintenanceTask lötspitzeReinigung;
    lötspitzeReinigung.name = "Lötspitze reinigen";
    lötspitzeReinigung.description = "Reinigung der Lötspitze mit feuchtem Schwamm";
    lötspitzeReinigung.intervalHours = 4;
    lötspitzeReinigung.priority = 4;
    addMaintenanceTask(lötspitzeReinigung);

    MaintenanceTask achsenSchmierung;
    achsenSchmierung.name = "Achsen schmieren";
    achsenSchmierung.description = "Schmierung aller beweglichen Achsen";
    achsenSchmierung.intervalHours = 168; // 1 Woche
    achsenSchmierung.priority = 3;
    addMaintenanceTask(achsenSchmierung);

    // Timer für regelmäßige Überprüfungen
    QTimer *checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, &MaintenanceSystem::checkMaintenanceSchedule);
    checkTimer->start(3600000); // Stündliche Überprüfung
}

void MaintenanceSystem::addMaintenanceTask(const MaintenanceTask &task) {
    tasks[task.name] = task;
    checkMaintenanceSchedule();
}

void MaintenanceSystem::updateTaskStatus(const QString &taskName, const QDateTime &performed) {
    if (tasks.contains(taskName)) {
        tasks[taskName].lastPerformed = performed;
        tasks[taskName].isRequired = false;
        
        // Wartungsereignis protokollieren
        logMaintenanceEvent(taskName, "Wartung durchgeführt");
    }
}

QVector<MaintenanceTask> MaintenanceSystem::getDueTasks() const {
    QVector<MaintenanceTask> dueTasks;
    QDateTime now = QDateTime::currentDateTime();
    
    for (const auto &task : tasks) {
        if (task.lastPerformed.isValid()) {
            qint64 hoursSinceLastMaintenance = task.lastPerformed.secsTo(now) / 3600;
            if (hoursSinceLastMaintenance >= task.intervalHours) {
                dueTasks.append(task);
            }
        } else {
            // Noch nie gewartet
            dueTasks.append(task);
        }
    }
    
    // Nach Priorität sortieren
    std::sort(dueTasks.begin(), dueTasks.end(),
              [](const MaintenanceTask &a, const MaintenanceTask &b) {
                  return a.priority > b.priority;
              });
    
    return dueTasks;
}

void MaintenanceSystem::updateComponentStatus(const QString &component, double wearLevel) {
    ComponentStatus &status = components[component];
    status.name = component;
    status.wearLevel = wearLevel;
    
    // Verschleißrate berechnen
    double wearRate = calculateWearRate(component);
    
    // Nächste Wartung vorhersagen
    int hoursUntilMaintenance = static_cast<int>((1.0 - wearLevel) / wearRate);
    status.nextMaintenance = QDateTime::currentDateTime().addSecs(hoursUntilMaintenance * 3600);
    
    // Warnung bei kritischem Verschleiß
    if (wearLevel > 0.8) {
        status.needsAttention = true;
        emit componentWearCritical(component, wearLevel);
    }
    
    // Wartungshistorie aktualisieren
    if (wearLevel > 0.9) {
        logMaintenanceEvent(component, "Kritischer Verschleiß festgestellt");
    }
}

ComponentStatus MaintenanceSystem::getComponentStatus(const QString &component) const {
    return components.value(component);
}

QVector<ComponentStatus> MaintenanceSystem::getAllComponentStatus() const {
    return QVector<ComponentStatus>(components.values());
}

double MaintenanceSystem::predictFailureProbability(const QString &component) const {
    if (!components.contains(component)) {
        return 0.0;
    }
    
    const ComponentStatus &status = components[component];
    double wearRate = calculateWearRate(component);
    double timeInUse = status.operatingHours;
    
    // Weibull-Verteilung für Ausfallwahrscheinlichkeit
    double shape = 2.0; // Form-Parameter
    double scale = 1000.0; // Skalierungs-Parameter
    
    return 1.0 - std::exp(-std::pow(timeInUse / scale, shape));
}

QDateTime MaintenanceSystem::estimateNextMaintenance(const QString &component) const {
    if (!components.contains(component)) {
        return QDateTime();
    }
    
    const ComponentStatus &status = components[component];
    double wearRate = calculateWearRate(component);
    
    if (wearRate <= 0) {
        return QDateTime();
    }
    
    // Stunden bis zum Erreichen des kritischen Verschleißlevels
    int hoursUntilMaintenance = static_cast<int>((0.8 - status.wearLevel) / wearRate);
    return QDateTime::currentDateTime().addSecs(hoursUntilMaintenance * 3600);
}

void MaintenanceSystem::logMaintenanceEvent(const QString &component, 
                                          const QString &action) {
    if (!maintenanceLog.contains(component)) {
        maintenanceLog[component] = QVector<QString>();
    }
    
    QString logEntry = QString("%1: %2")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
        .arg(action);
    
    maintenanceLog[component].append(logEntry);
}

QVector<QString> MaintenanceSystem::getMaintenanceHistory(const QString &component) const {
    return maintenanceLog.value(component);
}

void MaintenanceSystem::checkMaintenanceSchedule() {
    QVector<MaintenanceTask> dueTasks = getDueTasks();
    for (const auto &task : dueTasks) {
        if (!task.isRequired) {
            emit maintenanceRequired(task.name, 
                "Planmäßige Wartung fällig");
        }
    }
}

void MaintenanceSystem::analyzeWearPatterns() {
    for (const auto &component : components.keys()) {
        double wearRate = calculateWearRate(component);
        double failureProbability = predictFailureProbability(component);
        
        if (failureProbability > 0.7) {
            emit predictiveWarning(component, failureProbability);
        }
    }
}

double MaintenanceSystem::calculateWearRate(const QString &component) const {
    if (!components.contains(component)) {
        return 0.0;
    }
    
    const ComponentStatus &status = components[component];
    if (status.operatingHours == 0) {
        return 0.0;
    }
    
    return status.wearLevel / status.operatingHours;
}
