#include "security_system.h"
#include <QDebug>
#include <algorithm>

SecuritySystem::SecuritySystem(QObject *parent)
    : QObject(parent)
    , currentSmokeLevel(0.0)
    , smokeThreshold(50.0)
    , smokeDetectionEnabled(true)
{
    // Timer für Umgebungsüberwachung
    QTimer *monitorTimer = new QTimer(this);
    connect(monitorTimer, &QTimer::timeout, this, &SecuritySystem::monitorEnvironment);
    monitorTimer->start(100); // Alle 100ms aktualisieren
}

void SecuritySystem::addSecurityZone(const SecurityZone &zone) {
    zones.append(zone);
}

void SecuritySystem::removeSecurityZone(const QString &name) {
    zones.removeIf([name](const SecurityZone &zone) {
        return zone.name == name;
    });
}

bool SecuritySystem::isPositionInSafeZone(const QVector3D &position) const {
    for (const auto &zone : zones) {
        // Prüfen, ob die Position innerhalb der Zonengrenzen liegt
        bool inside = true;
        for (int i = 0; i < zone.boundaries.size(); i++) {
            QVector3D p1 = zone.boundaries[i];
            QVector3D p2 = zone.boundaries[(i + 1) % zone.boundaries.size()];
            
            // Punkt-in-Polygon-Test
            if ((position.y() - p1.y()) * (p2.x() - p1.x()) >
                (position.x() - p1.x()) * (p2.y() - p1.y())) {
                inside = false;
                break;
            }
        }
        
        if (inside) {
            return true;
        }
    }
    return false;
}

void SecuritySystem::enableSmokeDetection(bool enable) {
    smokeDetectionEnabled = enable;
}

double SecuritySystem::getSmokeLevel() const {
    return currentSmokeLevel;
}

void SecuritySystem::setSmokeThreshold(double threshold) {
    smokeThreshold = threshold;
}

void SecuritySystem::updateObstacleMap(const QVector<QVector3D> &obstacles) {
    // Hinderniskarte aktualisieren und Kollisionsprüfung vorbereiten
    // Hier könnte ein Octree oder ähnliche Datenstruktur verwendet werden
}

bool SecuritySystem::checkCollisionPath(const QVector3D &start, const QVector3D &end) {
    // Kollisionsprüfung entlang des Pfades
    QVector3D direction = end - start;
    float length = direction.length();
    direction.normalize();
    
    // Schrittweise entlang des Pfades prüfen
    const float step = 1.0f; // 1mm Schritte
    for (float dist = 0; dist < length; dist += step) {
        QVector3D point = start + direction * dist;
        if (!isPositionInSafeZone(point)) {
            return true; // Kollision gefunden
        }
    }
    
    return false;
}

double SecuritySystem::getMinimumDistance(const QVector3D &position) const {
    double minDist = std::numeric_limits<double>::max();
    
    // Minimalen Abstand zu allen Hindernissen berechnen
    for (const auto &zone : zones) {
        for (const auto &boundary : zone.boundaries) {
            double dist = (position - boundary).length();
            minDist = std::min(minDist, dist);
        }
    }
    
    return minDist;
}

bool SecuritySystem::authenticateUser(const QString &userId, const QString &password) {
    // Hier würde die tatsächliche Authentifizierung stattfinden
    // Für dieses Beispiel nur eine einfache Demonstration
    return password == "secure123";
}

void SecuritySystem::grantAccess(const QString &userId, const QString &zone) {
    for (auto &securityZone : zones) {
        if (securityZone.name == zone) {
            if (!securityZone.allowedUsers.contains(userId)) {
                securityZone.allowedUsers.append(userId);
            }
            break;
        }
    }
}

void SecuritySystem::revokeAccess(const QString &userId, const QString &zone) {
    for (auto &securityZone : zones) {
        if (securityZone.name == zone) {
            securityZone.allowedUsers.removeAll(userId);
            break;
        }
    }
}

void SecuritySystem::logSecurityEvent(const SecurityEvent &event) {
    eventLog.append(event);
    
    // Kritische Ereignisse sofort melden
    if (event.severity == "critical") {
        emit securityViolation(event.type, event.description);
    }
}

QVector<SecurityEvent> SecuritySystem::getSecurityLog(const QDateTime &start,
                                                    const QDateTime &end) const {
    QVector<SecurityEvent> filteredLog;
    
    for (const auto &event : eventLog) {
        if (event.timestamp >= start && event.timestamp <= end) {
            filteredLog.append(event);
        }
    }
    
    return filteredLog;
}

void SecuritySystem::checkZoneBoundaries(const QVector3D &position) {
    for (const auto &zone : zones) {
        if (!isPositionInSafeZone(position)) {
            emit zoneViolation(zone.name, position);
            
            SecurityEvent event;
            event.timestamp = QDateTime::currentDateTime();
            event.type = "zone_violation";
            event.description = QString("Position außerhalb der Zone %1").arg(zone.name);
            event.severity = "warning";
            event.location = position;
            
            logSecurityEvent(event);
        }
    }
}

void SecuritySystem::monitorEnvironment() {
    if (smokeDetectionEnabled) {
        // Rauchsensor auslesen (simuliert)
        currentSmokeLevel = 0.0;
        static double trend = 0.1;
        
        // Zufällige Schwankungen simulieren
        currentSmokeLevel += trend * (rand() % 100) / 100.0;
        if (currentSmokeLevel > 100.0 || currentSmokeLevel < 0.0) {
            trend = -trend;
        }
        
        if (currentSmokeLevel > smokeThreshold) {
            emit smokeDetected(currentSmokeLevel);
            
            SecurityEvent event;
            event.timestamp = QDateTime::currentDateTime();
            event.type = "smoke_detected";
            event.description = QString("Rauchentwicklung über Grenzwert: %1")
                                      .arg(currentSmokeLevel);
            event.severity = "critical";
            
            logSecurityEvent(event);
            emit emergencyStop("Rauchentwicklung");
        }
    }
}

void SecuritySystem::validateMovement(const QVector3D &target) {
    // Bewegung auf Sicherheit prüfen
    if (!isPositionInSafeZone(target)) {
        emit securityViolation("movement", "Zielposition außerhalb der sicheren Zone");
        return;
    }
    
    double distance = getMinimumDistance(target);
    if (distance < 50.0) { // 50mm Sicherheitsabstand
        emit collisionWarning(target, distance);
    }
}

bool SecuritySystem::isMovementAllowed(const QVector3D &start, 
                                     const QVector3D &end) const {
    // Bewegung auf Kollisionen prüfen
    if (checkCollisionPath(start, end)) {
        return false;
    }
    
    // Geschwindigkeitsbegrenzungen prüfen
    for (const auto &zone : zones) {
        if (isPositionInSafeZone(start) || isPositionInSafeZone(end)) {
            QVector3D movement = end - start;
            double speed = movement.length(); // Vereinfachte Geschwindigkeitsberechnung
            if (speed > zone.maxSpeed) {
                return false;
            }
        }
    }
    
    return true;
}

void SecuritySystem::updateSecurityStatus() {
    // Gesamtstatus des Sicherheitssystems aktualisieren
    bool systemOk = true;
    
    // Rauchmelder prüfen
    if (smokeDetectionEnabled && currentSmokeLevel > smokeThreshold) {
        systemOk = false;
    }
    
    // Zonenverletzungen prüfen
    for (const auto &event : eventLog) {
        if (event.severity == "critical" && 
            event.timestamp > QDateTime::currentDateTime().addSecs(-3600)) {
            systemOk = false;
            break;
        }
    }
    
    if (!systemOk) {
        emit securityViolation("system_check", "Sicherheitssystem meldet Probleme");
    }
}
