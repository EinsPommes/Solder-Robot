#ifndef SOLDERROBOT_MOTION_CONTROLLER_H
#define SOLDERROBOT_MOTION_CONTROLLER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>

class MotionController : public QObject {
    Q_OBJECT

public:
    explicit MotionController(QObject *parent = nullptr);
    ~MotionController();

    bool initialize();
    void moveToPosition(double x, double y, double z);
    void setConveyorSpeed(int speed);
    void emergencyStop();

signals:
    void positionChanged(double x, double y, double z);
    void conveyorSpeedChanged(int speed);
    void errorOccurred(const QString &error);

private:
    bool connectToHardware();
    void sendGCode(const QString &command);

    QSerialPort *serialPort;
    double currentX, currentY, currentZ;
    int currentConveyorSpeed;
    bool isInitialized;
};

#endif // SOLDERROBOT_MOTION_CONTROLLER_H
