#ifndef SOLDERROBOT_JOB_MANAGER_H
#define SOLDERROBOT_JOB_MANAGER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <opencv2/opencv.hpp>

// Struktur für einen einzelnen Lötpunkt
struct SolderPoint {
    QVector3D position;         // X, Y, Z Position
    double temperature;         // Löttemperatur in °C
    int dwellTime;             // Verweilzeit in ms
    QString type;              // "PTH", "SMD", etc.
    bool completed;            // Status
    QDateTime timestamp;       // Zeitstempel der Ausführung
};

// Struktur für eine Leiterplatte (PCB)
struct PCBData {
    QString name;              // Name oder ID der Platine
    QVector2D size;           // Größe in mm
    QVector2D origin;         // Referenzpunkt
    QString fiducialType;     // Art der Referenzmarken
    QVector<QPointF> fiducials; // Positionen der Referenzmarken
    cv::Mat image;            // Bild der Platine (optional)
};

// Struktur für einen Lötauftrag
struct SolderJob {
    QString id;               // Eindeutige Job-ID
    QString name;             // Beschreibender Name
    PCBData pcb;             // Platinendaten
    QVector<SolderPoint> points; // Liste der Lötpunkte
    int priority;            // Priorität (1-5)
    QDateTime created;       // Erstellungszeitpunkt
    QDateTime deadline;      // Deadline
    QString status;          // "waiting", "in_progress", "completed", "error"
};

class JobManager : public QObject {
    Q_OBJECT

public:
    explicit JobManager(QObject *parent = nullptr);

    // Job-Verwaltung
    QString createJob(const SolderJob &job);
    bool updateJob(const QString &jobId, const SolderJob &job);
    bool deleteJob(const QString &jobId);
    SolderJob getJob(const QString &jobId) const;
    QVector<SolderJob> getAllJobs() const;
    QVector<SolderJob> getPendingJobs() const;

    // Lötpunkt-Erkennung
    bool detectSolderPoints(const QString &jobId);
    bool validateSolderPoints(const QString &jobId);
    bool adjustSolderPoints(const QString &jobId, const QVector3D &offset);

    // Job-Ausführung
    bool startJob(const QString &jobId);
    bool pauseJob(const QString &jobId);
    bool resumeJob(const QString &jobId);
    bool abortJob(const QString &jobId);
    
    // Import/Export
    bool importFromGerber(const QString &filename);
    bool importFromCAD(const QString &filename);
    bool exportToFile(const QString &jobId, const QString &filename);

    // Platinenerkennung
    bool detectPCB(const QString &jobId);
    bool calibratePCB(const QString &jobId);
    QVector3D getPCBOffset(const QString &jobId) const;

signals:
    void jobCreated(const QString &jobId);
    void jobUpdated(const QString &jobId);
    void jobStarted(const QString &jobId);
    void jobCompleted(const QString &jobId);
    void jobError(const QString &jobId, const QString &error);
    void pointCompleted(const QString &jobId, int pointIndex);
    void progressUpdated(const QString &jobId, int current, int total);
    void pcbDetected(const QString &jobId, const PCBData &pcbData);
    void calibrationRequired(const QString &jobId);

private:
    QMap<QString, SolderJob> jobs;
    QString currentJobId;
    bool isJobRunning;

    // Hilfsfunktionen
    bool validateJob(const SolderJob &job) const;
    void updateJobStatus(const QString &jobId, const QString &status);
    QVector<QPointF> detectFiducials(const cv::Mat &image);
    bool calculatePCBTransform(const PCBData &pcb, QTransform &transform);
    void optimizePointSequence(QVector<SolderPoint> &points);
};

#endif // SOLDERROBOT_JOB_MANAGER_H
