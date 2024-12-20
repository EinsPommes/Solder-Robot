#ifndef SOLDERROBOT_TEMPERATURE_CONTROL_H
#define SOLDERROBOT_TEMPERATURE_CONTROL_H

#include <QObject>
#include <QTimer>

class PIDController {
public:
    PIDController(double kp, double ki, double kd);
    double calculate(double setpoint, double processVariable);

private:
    double kp, ki, kd;
    double lastError;
    double integral;
};

class TemperatureControl : public QObject {
    Q_OBJECT

public:
    explicit TemperatureControl(QObject *parent = nullptr);
    ~TemperatureControl();

    bool initialize();
    void setTargetTemperature(double temperature);
    double getCurrentTemperature() const;
    void enableHeating(bool enable);

signals:
    void temperatureChanged(double temperature);
    void temperatureError(const QString &error);

private slots:
    void updateTemperature();

private:
    void applyHeatingPower(double power);
    double readTemperatureSensor();

    QTimer *updateTimer;
    PIDController *pidController;
    double targetTemperature;
    double currentTemperature;
    bool heatingEnabled;
};

#endif // SOLDERROBOT_TEMPERATURE_CONTROL_H
