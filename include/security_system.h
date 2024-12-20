#ifndef SOLDERROBOT_SECURITY_SYSTEM_H
#define SOLDERROBOT_SECURITY_SYSTEM_H

#include <QObject>
#include <QVector3D>
#include <QDateTime>

struct SecurityZone {
    QString name;
    QVector<QVector3D> boundaries;
    double maxSpeed;
    bool requiresAuthorization;
    QStringList allowedUsers;
};

struct SecurityEvent {
    QDateTime timestamp;
    QString type;
    QString description;
    QString severity;
    QVector3D location;
    QString userId;
};

class SecuritySystem : public QObject {
    Q_OBJECT

public:
    explicit SecuritySystem(QObject *parent = nullptr);
    
    // Zonenverwaltung
    void addSecurityZone(const SecurityZone &zone);
    void removeSecurityZone(const QString &name);
    bool isPositionInSafeZone(const QVector3D &position) const;
    
    // Rauchmelder
    void enableSmokeDetection(bool enable);
    double getSmokeLevel() const;
    void setSmokeThreshold(double threshold);
    
    // Kollisionserkennung
    void updateObstacleMap(const QVector<QVector3D> &obstacles);
    bool checkCollisionPath(const QVector3D &start, const QVector3D &end);
    double getMinimumDistance(const QVector3D &position) const;
    
    // Zugriffskontrolle
    bool authenticateUser(const QString &userId, const QString &password);
    void grantAccess(const QString &userId, const QString &zone);
    void revokeAccess(const QString &userId, const QString &zone);
    
    // Ereignisprotokollierung
    void logSecurityEvent(const SecurityEvent &event);
    QVector<SecurityEvent> getSecurityLog(const QDateTime &start,
                                        const QDateTime &end) const;
    
signals:
    void securityViolation(const QString &type, const QString &description);
    void smokeDetected(double level);
    void collisionWarning(const QVector3D &position, double distance);
    void unauthorizedAccess(const QString &userId, const QString &zone);
    void emergencyStop(const QString &reason);
    void zoneViolation(const QString &zoneName, const QVector3D &position);

private:
    QVector<SecurityZone> zones;
    QVector<SecurityEvent> eventLog;
    double currentSmokeLevel;
    double smokeThreshold;
    bool smokeDetectionEnabled;
    
    void checkZoneBoundaries(const QVector3D &position);
    void monitorEnvironment();
    void validateMovement(const QVector3D &target);
    bool isMovementAllowed(const QVector3D &start, const QVector3D &end) const;
    void updateSecurityStatus();
};

#endif // SOLDERROBOT_SECURITY_SYSTEM_H
