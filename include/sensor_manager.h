#ifndef SOLDERROBOT_SENSOR_MANAGER_H
#define SOLDERROBOT_SENSOR_MANAGER_H

#include <QObject>
#include <QTimer>

class SensorManager : public QObject {
    Q_OBJECT

public:
    explicit SensorManager(QObject *parent = nullptr);
    ~SensorManager();

    bool initialize();
    bool isObstacleDetected() const;
    bool isComponentPresent() const;
    double getLidarDistance() const;
    double getMagneticFieldStrength() const;

signals:
    void obstacleDetected(bool detected);
    void componentDetected(bool detected);
    void sensorError(const QString &error);

private slots:
    void updateSensorReadings();

private:
    bool readLidarSensor();
    bool readMagneticSensor();

    QTimer *updateTimer;
    double lidarDistance;
    double magneticFieldStrength;
    bool obstaclePresent;
    bool componentPresent;
};

#endif // SOLDERROBOT_SENSOR_MANAGER_H
