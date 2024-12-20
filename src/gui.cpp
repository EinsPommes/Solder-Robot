#include "gui.h"
#include "motion_controller.h"
#include "sensor_manager.h"
#include "temperature_control.h"
#include <QMessageBox>
#include <QGroupBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , motionController(new MotionController(this))
    , sensorManager(new SensorManager(this))
    , temperatureControl(new TemperatureControl(this))
{
    setupUI();

    // Verbindungen für Sicherheitsfunktionen
    connect(sensorManager, &SensorManager::obstacleDetected, this, [this](bool detected) {
        if (detected) {
            emergencyStop();
            QMessageBox::warning(this, "Sicherheitswarnung", 
                "Hindernis erkannt! Bewegung gestoppt.");
        }
    });

    // Initialisierung der Komponenten
    if (!motionController->initialize()) {
        QMessageBox::critical(this, "Fehler", 
            "Bewegungssteuerung konnte nicht initialisiert werden.");
    }

    if (!temperatureControl->initialize()) {
        QMessageBox::critical(this, "Fehler", 
            "Temperatursteuerung konnte nicht initialisiert werden.");
    }

    if (!sensorManager->initialize()) {
        QMessageBox::critical(this, "Fehler", 
            "Sensoren konnten nicht initialisiert werden.");
    }
}

MainWindow::~MainWindow() {
    // Qt übernimmt die Speicherbereinigung durch das Parent-Child-System
}

void MainWindow::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    auto mainLayout = new QVBoxLayout(centralWidget);
    
    // Achsensteuerung erstellen
    createAxisControls();
    // Temperatursteuerung erstellen
    createTemperatureControls();
    // Fließbandsteuerung erstellen
    createConveyorControls();
    // Sensoranzeige erstellen
    createSensorDisplay();
    
    // Not-Aus-Knopf
    emergencyStopButton = new QPushButton("NOT-AUS", this);
    emergencyStopButton->setStyleSheet("background-color: red; color: white;");
    emergencyStopButton->setMinimumHeight(50);
    connect(emergencyStopButton, &QPushButton::clicked, this, &MainWindow::emergencyStop);
    
    mainLayout->addWidget(emergencyStopButton);
    
    setWindowTitle("Lötroboter Steuerung");
    resize(800, 600);
}

void MainWindow::createAxisControls() {
    auto groupBox = new QGroupBox("Achsensteuerung", centralWidget);
    auto layout = new QGridLayout(groupBox);
    
    // X-Achse
    xPosDisplay = new QLCDNumber(this);
    auto xLabel = new QLabel("X-Position:", this);
    auto xSpinBox = new QDoubleSpinBox(this);
    xSpinBox->setRange(-1000, 1000);
    
    // Y-Achse
    yPosDisplay = new QLCDNumber(this);
    auto yLabel = new QLabel("Y-Position:", this);
    auto ySpinBox = new QDoubleSpinBox(this);
    ySpinBox->setRange(-1000, 1000);
    
    // Z-Achse
    zPosDisplay = new QLCDNumber(this);
    auto zLabel = new QLabel("Z-Position:", this);
    auto zSpinBox = new QDoubleSpinBox(this);
    zSpinBox->setRange(-1000, 1000);
    
    // Layout aufbauen
    layout->addWidget(xLabel, 0, 0);
    layout->addWidget(xPosDisplay, 0, 1);
    layout->addWidget(xSpinBox, 0, 2);
    
    layout->addWidget(yLabel, 1, 0);
    layout->addWidget(yPosDisplay, 1, 1);
    layout->addWidget(ySpinBox, 1, 2);
    
    layout->addWidget(zLabel, 2, 0);
    layout->addWidget(zPosDisplay, 2, 1);
    layout->addWidget(zSpinBox, 2, 2);
    
    // Bewegungssteuerung verbinden
    connect(xSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this, ySpinBox, zSpinBox](double x) {
        motionController->moveToPosition(x, ySpinBox->value(), zSpinBox->value());
    });
    
    static_cast<QVBoxLayout*>(centralWidget->layout())->addWidget(groupBox);
}

void MainWindow::createTemperatureControls() {
    auto groupBox = new QGroupBox("Temperatursteuerung", centralWidget);
    auto layout = new QHBoxLayout(groupBox);
    
    temperatureDisplay = new QLCDNumber(this);
    auto tempLabel = new QLabel("Temperatur (°C):", this);
    auto tempSpinBox = new QDoubleSpinBox(this);
    tempSpinBox->setRange(0, 450);
    tempSpinBox->setValue(350); // Standard-Löttemperatur
    
    layout->addWidget(tempLabel);
    layout->addWidget(temperatureDisplay);
    layout->addWidget(tempSpinBox);
    
    connect(tempSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            temperatureControl, &TemperatureControl::setTargetTemperature);
    
    static_cast<QVBoxLayout*>(centralWidget->layout())->addWidget(groupBox);
}

void MainWindow::createConveyorControls() {
    auto groupBox = new QGroupBox("Fließbandsteuerung", centralWidget);
    auto layout = new QHBoxLayout(groupBox);
    
    conveyorSpeedSlider = new QSlider(Qt::Horizontal, this);
    conveyorSpeedSlider->setRange(0, 100);
    
    auto speedLabel = new QLabel("Geschwindigkeit:", this);
    auto speedDisplay = new QLCDNumber(this);
    
    layout->addWidget(speedLabel);
    layout->addWidget(conveyorSpeedSlider);
    layout->addWidget(speedDisplay);
    
    connect(conveyorSpeedSlider, &QSlider::valueChanged, 
            [this, speedDisplay](int value) {
        speedDisplay->display(value);
        motionController->setConveyorSpeed(value);
    });
    
    static_cast<QVBoxLayout*>(centralWidget->layout())->addWidget(groupBox);
}

void MainWindow::createSensorDisplay() {
    auto groupBox = new QGroupBox("Sensorstatus", centralWidget);
    auto layout = new QGridLayout(groupBox);
    
    auto lidarLabel = new QLabel("LiDAR Abstand:", this);
    auto lidarDisplay = new QLCDNumber(this);
    
    auto magnetLabel = new QLabel("Magnetfeld:", this);
    auto magnetDisplay = new QLCDNumber(this);
    
    layout->addWidget(lidarLabel, 0, 0);
    layout->addWidget(lidarDisplay, 0, 1);
    layout->addWidget(magnetLabel, 1, 0);
    layout->addWidget(magnetDisplay, 1, 1);
    
    // Aktualisierung der Sensorwerte
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, [=]() {
        lidarDisplay->display(sensorManager->getLidarDistance());
        magnetDisplay->display(sensorManager->getMagneticFieldStrength());
    });
    updateTimer->start(100); // Alle 100ms aktualisieren
    
    static_cast<QVBoxLayout*>(centralWidget->layout())->addWidget(groupBox);
}

void MainWindow::updatePosition(double x, double y, double z) {
    xPosDisplay->display(x);
    yPosDisplay->display(y);
    zPosDisplay->display(z);
}

void MainWindow::updateTemperature(double temp) {
    temperatureDisplay->display(temp);
}

void MainWindow::updateConveyorSpeed(int speed) {
    conveyorSpeedSlider->setValue(speed);
}

void MainWindow::emergencyStop() {
    motionController->emergencyStop();
    temperatureControl->enableHeating(false);
    QMessageBox::warning(this, "Not-Aus", 
        "Not-Aus wurde aktiviert. Bitte überprüfen Sie die Anlage.");
}
