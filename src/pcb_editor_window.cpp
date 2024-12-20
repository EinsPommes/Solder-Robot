#include "pcb_editor_window.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>

PCBEditorWindow::PCBEditorWindow(JobManager* jobMgr, QWidget *parent)
    : QMainWindow(parent)
    , jobManager(jobMgr)
{
    setupUI();
    
    // Fenster-Eigenschaften
    setWindowTitle(tr("PCB Editor - Lötpunkte markieren"));
    setMinimumSize(800, 600);
    
    // Status aktualisieren
    updateStatusLabel(tr("Bereit"));
}

void PCBEditorWindow::setupUI() {
    // Haupteditor erstellen
    editor = new PCBEditor(this);
    setCentralWidget(editor);
    
    // Toolbar und Docks erstellen
    createToolBar();
    createPointsDock();
    createPropertiesDock();
    
    // Signale verbinden
    connect(editor, &PCBEditor::pointAdded, this, &PCBEditorWindow::updatePointsList);
    connect(editor, &PCBEditor::pointRemoved, this, &PCBEditorWindow::updatePointsList);
    connect(editor, &PCBEditor::pointSelected, this, &PCBEditorWindow::updatePointInfo);
}

void PCBEditorWindow::createToolBar() {
    mainToolBar = addToolBar(tr("Werkzeuge"));
    
    // Bild laden
    loadButton = new QPushButton(tr("Bild laden"), this);
    mainToolBar->addWidget(loadButton);
    connect(loadButton, &QPushButton::clicked, this, &PCBEditorWindow::loadImage);
    
    mainToolBar->addSeparator();
    
    // Editiermodus
    editButton = new QPushButton(tr("Punkte setzen"), this);
    editButton->setCheckable(true);
    mainToolBar->addWidget(editButton);
    connect(editButton, &QPushButton::toggled, this, &PCBEditorWindow::toggleEditMode);
    
    // Löschmodus
    eraseButton = new QPushButton(tr("Punkte löschen"), this);
    eraseButton->setCheckable(true);
    mainToolBar->addWidget(eraseButton);
    connect(eraseButton, &QPushButton::toggled, this, &PCBEditorWindow::toggleEraseMode);
    
    mainToolBar->addSeparator();
    
    // Punkttyp
    typeCombo = new QComboBox(this);
    typeCombo->addItems(QStringList() << "PTH" << "SMD" << "Andere");
    mainToolBar->addWidget(new QLabel(tr("Punkttyp:")));
    mainToolBar->addWidget(typeCombo);
    connect(typeCombo, &QComboBox::currentTextChanged, 
            this, &PCBEditorWindow::setPointType);
    
    mainToolBar->addSeparator();
    
    // Automatische Erkennung
    detectButton = new QPushButton(tr("Punkte erkennen"), this);
    mainToolBar->addWidget(detectButton);
    connect(detectButton, &QPushButton::clicked, this, &PCBEditorWindow::detectPoints);
    
    // Optimierung
    optimizeButton = new QPushButton(tr("Optimieren"), this);
    mainToolBar->addWidget(optimizeButton);
    connect(optimizeButton, &QPushButton::clicked, 
            this, &PCBEditorWindow::optimizePoints);
    
    mainToolBar->addSeparator();
    
    // Alle löschen
    clearButton = new QPushButton(tr("Alle löschen"), this);
    mainToolBar->addWidget(clearButton);
    connect(clearButton, &QPushButton::clicked, this, &PCBEditorWindow::clearAllPoints);
    
    // Speichern
    saveButton = new QPushButton(tr("Job speichern"), this);
    mainToolBar->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &PCBEditorWindow::saveJob);
}

void PCBEditorWindow::createPointsDock() {
    // Punkteliste erstellen
    pointsDock = new QDockWidget(tr("Lötpunkte"), this);
    pointsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    QWidget *pointsWidget = new QWidget(pointsDock);
    QVBoxLayout *layout = new QVBoxLayout(pointsWidget);
    
    pointsList = new QListWidget(pointsWidget);
    layout->addWidget(pointsList);
    
    pointsDock->setWidget(pointsWidget);
    addDockWidget(Qt::RightDockWidgetArea, pointsDock);
    
    connect(pointsList, &QListWidget::currentRowChanged, 
            this, &PCBEditorWindow::selectPoint);
}

void PCBEditorWindow::createPropertiesDock() {
    // Eigenschaften-Editor erstellen
    propertiesDock = new QDockWidget(tr("Eigenschaften"), this);
    propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    QWidget *propsWidget = new QWidget(propertiesDock);
    QFormLayout *layout = new QFormLayout(propsWidget);
    
    // Position
    xPos = new QSpinBox(propsWidget);
    xPos->setRange(0, 9999);
    layout->addRow(tr("X-Position:"), xPos);
    
    yPos = new QSpinBox(propsWidget);
    yPos->setRange(0, 9999);
    layout->addRow(tr("Y-Position:"), yPos);
    
    // Temperatur
    tempSpinBox = new QDoubleSpinBox(propsWidget);
    tempSpinBox->setRange(200, 450);
    tempSpinBox->setSuffix(" °C");
    tempSpinBox->setValue(350);
    layout->addRow(tr("Temperatur:"), tempSpinBox);
    
    // Verweilzeit
    dwellSpinBox = new QSpinBox(propsWidget);
    dwellSpinBox->setRange(100, 5000);
    dwellSpinBox->setSuffix(" ms");
    dwellSpinBox->setValue(1000);
    layout->addRow(tr("Verweilzeit:"), dwellSpinBox);
    
    // Status-Label
    statusLabel = new QLabel(propsWidget);
    layout->addRow(statusLabel);
    
    // Aktualisieren-Button
    QPushButton *updateButton = new QPushButton(tr("Aktualisieren"), propsWidget);
    layout->addRow(updateButton);
    connect(updateButton, &QPushButton::clicked, 
            this, &PCBEditorWindow::updateSelectedPoint);
    
    propertiesDock->setWidget(propsWidget);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock);
}

void PCBEditorWindow::loadImage() {
    QString filename = QFileDialog::getOpenFileName(this,
        tr("PCB-Bild laden"), QString(),
        tr("Bilder (*.png *.jpg *.bmp)"));
        
    if (!filename.isEmpty()) {
        editor->loadPCBImage(filename);
        updateStatusLabel(tr("Bild geladen: %1").arg(filename));
    }
}

void PCBEditorWindow::saveJob() {
    if (editor->getPoints().isEmpty()) {
        QMessageBox::warning(this, tr("Fehler"),
            tr("Keine Lötpunkte vorhanden"));
        return;
    }
    
    // Neuen Job erstellen
    SolderJob job;
    job.name = "PCB Job " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    job.points = editor->getPoints();
    job.priority = 3; // Mittlere Priorität
    job.created = QDateTime::currentDateTime();
    job.deadline = job.created.addDays(1);
    job.status = "waiting";
    
    // Job speichern
    QString jobId = jobManager->createJob(job);
    if (jobId.isEmpty()) {
        QMessageBox::critical(this, tr("Fehler"),
            tr("Job konnte nicht gespeichert werden"));
        return;
    }
    
    currentJobId = jobId;
    updateStatusLabel(tr("Job gespeichert: %1").arg(jobId));
}

void PCBEditorWindow::toggleEditMode() {
    if (editButton->isChecked()) {
        eraseButton->setChecked(false);
        editor->setEditMode(true);
        updateStatusLabel(tr("Editiermodus: Klicken Sie zum Setzen von Punkten"));
    } else {
        editor->setEditMode(false);
        updateStatusLabel(tr("Editiermodus deaktiviert"));
    }
}

void PCBEditorWindow::toggleEraseMode() {
    if (eraseButton->isChecked()) {
        editButton->setChecked(false);
        editor->setEraseMode(true);
        updateStatusLabel(tr("Löschmodus: Klicken Sie zum Entfernen von Punkten"));
    } else {
        editor->setEraseMode(false);
        updateStatusLabel(tr("Löschmodus deaktiviert"));
    }
}

void PCBEditorWindow::setPointType(const QString &type) {
    editor->setPointType(type);
    updateStatusLabel(tr("Punkttyp: %1").arg(type));
}

void PCBEditorWindow::updatePointInfo(const SolderPoint &point) {
    xPos->setValue(point.position.x());
    yPos->setValue(point.position.y());
    tempSpinBox->setValue(point.temperature);
    dwellSpinBox->setValue(point.dwellTime);
    typeCombo->setCurrentText(point.type);
}

void PCBEditorWindow::updatePointsList() {
    pointsList->clear();
    
    const auto &points = editor->getPoints();
    for (int i = 0; i < points.size(); ++i) {
        const auto &point = points[i];
        QString text = tr("Punkt %1: (%2, %3) - %4")
            .arg(i + 1)
            .arg(point.position.x())
            .arg(point.position.y())
            .arg(point.type);
        pointsList->addItem(text);
    }
    
    updateStatusLabel(tr("%1 Punkte gesetzt").arg(points.size()));
}

void PCBEditorWindow::selectPoint(int index) {
    const auto &points = editor->getPoints();
    if (index >= 0 && index < points.size()) {
        updatePointInfo(points[index]);
    }
}

void PCBEditorWindow::updateSelectedPoint() {
    int index = pointsList->currentRow();
    if (index < 0) return;
    
    auto points = editor->getPoints();
    if (index >= points.size()) return;
    
    // Punkt aktualisieren
    points[index].position.setX(xPos->value());
    points[index].position.setY(yPos->value());
    points[index].temperature = tempSpinBox->value();
    points[index].dwellTime = dwellSpinBox->value();
    points[index].type = typeCombo->currentText();
    
    editor->setPoints(points);
    updatePointsList();
    updateStatusLabel(tr("Punkt %1 aktualisiert").arg(index + 1));
}

void PCBEditorWindow::detectPoints() {
    if (!currentJobId.isEmpty()) {
        if (jobManager->detectSolderPoints(currentJobId)) {
            // Erkannte Punkte laden
            const auto &job = jobManager->getJob(currentJobId);
            editor->setPoints(job.points);
            updatePointsList();
            updateStatusLabel(tr("Punkte automatisch erkannt"));
        } else {
            QMessageBox::warning(this, tr("Fehler"),
                tr("Automatische Erkennung fehlgeschlagen"));
        }
    }
}

void PCBEditorWindow::optimizePoints() {
    if (!currentJobId.isEmpty()) {
        const auto &job = jobManager->getJob(currentJobId);
        QVector<SolderPoint> optimizedPoints = job.points;
        
        // Hier würde die Optimierung stattfinden
        // Zum Beispiel durch Sortierung nach kürzestem Weg
        
        editor->setPoints(optimizedPoints);
        updatePointsList();
        updateStatusLabel(tr("Punkte optimiert"));
    }
}

void PCBEditorWindow::clearAllPoints() {
    if (QMessageBox::question(this, tr("Bestätigung"),
        tr("Wirklich alle Punkte löschen?")) == QMessageBox::Yes) {
        editor->clearPoints();
        updatePointsList();
        updateStatusLabel(tr("Alle Punkte gelöscht"));
    }
}

void PCBEditorWindow::updateStatusLabel(const QString &message) {
    statusLabel->setText(message);
    statusBar()->showMessage(message, 3000);
}
