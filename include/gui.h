#ifndef SOLDERROBOT_GUI_H
#define SOLDERROBOT_GUI_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QSlider>
#include <QLCDNumber>

class MotionController;
class SensorManager;
class TemperatureControl;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updatePosition(double x, double y, double z);
    void updateTemperature(double temp);
    void updateConveyorSpeed(int speed);
    void emergencyStop();

private:
    void setupUI();
    void createAxisControls();
    void createTemperatureControls();
    void createConveyorControls();
    void createSensorDisplay();

    // UI Components
    QWidget *centralWidget;
    QLCDNumber *xPosDisplay, *yPosDisplay, *zPosDisplay;
    QLCDNumber *temperatureDisplay;
    QSlider *conveyorSpeedSlider;
    QPushButton *emergencyStopButton;
    
    // System Components
    MotionController *motionController;
    SensorManager *sensorManager;
    TemperatureControl *temperatureControl;
};

#endif // SOLDERROBOT_GUI_H
