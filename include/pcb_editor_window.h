#ifndef SOLDERROBOT_PCB_EDITOR_WINDOW_H
#define SOLDERROBOT_PCB_EDITOR_WINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QDockWidget>
#include <QListWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "pcb_editor.h"
#include "job_manager.h"

class PCBEditorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit PCBEditorWindow(JobManager* jobManager, QWidget *parent = nullptr);

private slots:
    void loadImage();
    void saveJob();
    void toggleEditMode();
    void toggleEraseMode();
    void setPointType(const QString &type);
    void updatePointInfo(const SolderPoint &point);
    void updatePointsList();
    void selectPoint(int index);
    void updateSelectedPoint();
    void detectPoints();
    void optimizePoints();
    void clearAllPoints();

private:
    // Hauptkomponenten
    PCBEditor *editor;
    JobManager *jobManager;
    QString currentJobId;

    // Toolbars und Docks
    QToolBar *mainToolBar;
    QDockWidget *pointsDock;
    QDockWidget *propertiesDock;

    // Steuerelemente
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *editButton;
    QPushButton *eraseButton;
    QPushButton *detectButton;
    QPushButton *optimizeButton;
    QPushButton *clearButton;
    QComboBox *typeCombo;

    // Punkteliste
    QListWidget *pointsList;

    // Eigenschaften-Editor
    QSpinBox *xPos;
    QSpinBox *yPos;
    QDoubleSpinBox *tempSpinBox;
    QSpinBox *dwellSpinBox;
    QLabel *statusLabel;

    void setupUI();
    void createToolBar();
    void createPointsDock();
    void createPropertiesDock();
    void updateStatusLabel(const QString &message);
};

#endif // SOLDERROBOT_PCB_EDITOR_WINDOW_H
