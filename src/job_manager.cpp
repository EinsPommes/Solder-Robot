#include "job_manager.h"
#include <QUuid>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

JobManager::JobManager(QObject *parent)
    : QObject(parent)
    , isJobRunning(false)
{
}

QString JobManager::createJob(const SolderJob &job) {
    if (!validateJob(job)) {
        qDebug() << "Ungültiger Job";
        return QString();
    }

    // Neue Job-ID generieren
    QString jobId = QUuid::createUuid().toString();
    
    // Job mit Status "waiting" speichern
    SolderJob newJob = job;
    newJob.id = jobId;
    newJob.status = "waiting";
    newJob.created = QDateTime::currentDateTime();
    
    jobs[jobId] = newJob;
    emit jobCreated(jobId);
    
    return jobId;
}

bool JobManager::updateJob(const QString &jobId, const SolderJob &job) {
    if (!jobs.contains(jobId)) {
        return false;
    }

    if (!validateJob(job)) {
        return false;
    }

    jobs[jobId] = job;
    emit jobUpdated(jobId);
    return true;
}

bool JobManager::deleteJob(const QString &jobId) {
    if (!jobs.contains(jobId)) {
        return false;
    }

    if (jobId == currentJobId && isJobRunning) {
        return false; // Laufende Jobs können nicht gelöscht werden
    }

    jobs.remove(jobId);
    return true;
}

SolderJob JobManager::getJob(const QString &jobId) const {
    return jobs.value(jobId);
}

QVector<SolderJob> JobManager::getAllJobs() const {
    return QVector<SolderJob>(jobs.values());
}

QVector<SolderJob> JobManager::getPendingJobs() const {
    QVector<SolderJob> pendingJobs;
    for (const auto &job : jobs) {
        if (job.status == "waiting" || job.status == "in_progress") {
            pendingJobs.append(job);
        }
    }
    
    // Nach Priorität und Deadline sortieren
    std::sort(pendingJobs.begin(), pendingJobs.end(), 
              [](const SolderJob &a, const SolderJob &b) {
                  if (a.priority != b.priority) {
                      return a.priority > b.priority;
                  }
                  return a.deadline < b.deadline;
              });
    
    return pendingJobs;
}

bool JobManager::detectSolderPoints(const QString &jobId) {
    if (!jobs.contains(jobId)) {
        return false;
    }

    SolderJob &job = jobs[jobId];
    
    // Bild der Platine laden und verarbeiten
    cv::Mat image = job.pcb.image;
    if (image.empty()) {
        return false;
    }
    
    // Bildvorverarbeitung
    cv::Mat processed;
    cv::cvtColor(image, processed, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(processed, processed, cv::Size(5, 5), 0);
    
    // Lötpunkte erkennen (Kreiserkennung)
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(processed, circles, cv::HOUGH_GRADIENT, 1, 20,
                     50, 30, 1, 30);
    
    // Gefundene Kreise in Lötpunkte umwandeln
    job.points.clear();
    for (const auto &circle : circles) {
        SolderPoint point;
        point.position = QVector3D(circle[0], circle[1], 0); // Z wird später kalibriert
        point.temperature = 350.0; // Standard-Löttemperatur
        point.dwellTime = 1000;   // Standard-Verweilzeit
        point.type = "PTH";       // Standard-Typ
        point.completed = false;
        job.points.append(point);
    }
    
    return !job.points.isEmpty();
}

bool JobManager::validateSolderPoints(const QString &jobId) {
    if (!jobs.contains(jobId)) {
        return false;
    }

    const SolderJob &job = jobs[jobId];
    
    // Prüfen, ob alle Punkte innerhalb der Platinengrenzen liegen
    for (const auto &point : job.points) {
        if (point.position.x() < 0 || point.position.x() > job.pcb.size.x() ||
            point.position.y() < 0 || point.position.y() > job.pcb.size.y()) {
            return false;
        }
        
        // Prüfen, ob die Temperatur im gültigen Bereich liegt
        if (point.temperature < 200 || point.temperature > 450) {
            return false;
        }
        
        // Prüfen, ob die Verweilzeit sinnvoll ist
        if (point.dwellTime < 100 || point.dwellTime > 5000) {
            return false;
        }
    }
    
    return true;
}

bool JobManager::adjustSolderPoints(const QString &jobId, const QVector3D &offset) {
    if (!jobs.contains(jobId)) {
        return false;
    }

    SolderJob &job = jobs[jobId];
    
    // Alle Punkte um den Offset verschieben
    for (auto &point : job.points) {
        point.position += offset;
    }
    
    return validateSolderPoints(jobId);
}

bool JobManager::startJob(const QString &jobId) {
    if (!jobs.contains(jobId) || isJobRunning) {
        return false;
    }

    SolderJob &job = jobs[jobId];
    if (job.status != "waiting") {
        return false;
    }

    // PCB-Erkennung und Kalibrierung durchführen
    if (!detectPCB(jobId)) {
        emit jobError(jobId, "PCB konnte nicht erkannt werden");
        return false;
    }

    // Optimale Reihenfolge der Lötpunkte berechnen
    optimizePointSequence(job.points);

    currentJobId = jobId;
    isJobRunning = true;
    updateJobStatus(jobId, "in_progress");
    emit jobStarted(jobId);

    return true;
}

bool JobManager::pauseJob(const QString &jobId) {
    if (jobId != currentJobId || !isJobRunning) {
        return false;
    }

    isJobRunning = false;
    updateJobStatus(jobId, "paused");
    return true;
}

bool JobManager::resumeJob(const QString &jobId) {
    if (jobId != currentJobId || isJobRunning) {
        return false;
    }

    isJobRunning = true;
    updateJobStatus(jobId, "in_progress");
    return true;
}

bool JobManager::abortJob(const QString &jobId) {
    if (jobId != currentJobId || !isJobRunning) {
        return false;
    }

    isJobRunning = false;
    currentJobId.clear();
    updateJobStatus(jobId, "aborted");
    return true;
}

bool JobManager::importFromGerber(const QString &filename) {
    // Hier würde die Gerber-Datei-Verarbeitung implementiert werden
    // Dies erfordert eine spezielle Gerber-Parser-Bibliothek
    return false;
}

bool JobManager::importFromCAD(const QString &filename) {
    // Hier würde der CAD-Datei-Import implementiert werden
    // Dies erfordert entsprechende CAD-Format-Parser
    return false;
}

bool JobManager::exportToFile(const QString &jobId, const QString &filename) {
    if (!jobs.contains(jobId)) {
        return false;
    }

    const SolderJob &job = jobs[jobId];
    
    QJsonObject jobObject;
    jobObject["id"] = job.id;
    jobObject["name"] = job.name;
    jobObject["priority"] = job.priority;
    jobObject["created"] = job.created.toString(Qt::ISODate);
    jobObject["deadline"] = job.deadline.toString(Qt::ISODate);
    jobObject["status"] = job.status;
    
    // PCB-Daten
    QJsonObject pcbObject;
    pcbObject["name"] = job.pcb.name;
    pcbObject["size_x"] = job.pcb.size.x();
    pcbObject["size_y"] = job.pcb.size.y();
    pcbObject["origin_x"] = job.pcb.origin.x();
    pcbObject["origin_y"] = job.pcb.origin.y();
    
    // Lötpunkte
    QJsonArray pointsArray;
    for (const auto &point : job.points) {
        QJsonObject pointObject;
        pointObject["x"] = point.position.x();
        pointObject["y"] = point.position.y();
        pointObject["z"] = point.position.z();
        pointObject["temperature"] = point.temperature;
        pointObject["dwell_time"] = point.dwellTime;
        pointObject["type"] = point.type;
        pointObject["completed"] = point.completed;
        pointsArray.append(pointObject);
    }
    
    jobObject["pcb"] = pcbObject;
    jobObject["points"] = pointsArray;
    
    QJsonDocument doc(jobObject);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

bool JobManager::detectPCB(const QString &jobId) {
    if (!jobs.contains(jobId)) {
        return false;
    }

    SolderJob &job = jobs[jobId];
    
    // Referenzmarken erkennen
    job.pcb.fiducials = detectFiducials(job.pcb.image);
    if (job.pcb.fiducials.isEmpty()) {
        return false;
    }
    
    // PCB-Position und -Ausrichtung berechnen
    QTransform transform;
    if (!calculatePCBTransform(job.pcb, transform)) {
        return false;
    }
    
    // Lötpunkte entsprechend transformieren
    for (auto &point : job.points) {
        QPointF transformed = transform.map(QPointF(point.position.x(), 
                                                  point.position.y()));
        point.position.setX(transformed.x());
        point.position.setY(transformed.y());
    }
    
    emit pcbDetected(jobId, job.pcb);
    return true;
}

bool JobManager::calibratePCB(const QString &jobId) {
    if (!jobs.contains(jobId)) {
        return false;
    }

    // Kalibrierung durchführen
    emit calibrationRequired(jobId);
    return true;
}

QVector3D JobManager::getPCBOffset(const QString &jobId) const {
    if (!jobs.contains(jobId)) {
        return QVector3D();
    }

    const PCBData &pcb = jobs[jobId].pcb;
    return QVector3D(pcb.origin.x(), pcb.origin.y(), 0);
}

bool JobManager::validateJob(const SolderJob &job) const {
    if (job.name.isEmpty()) {
        return false;
    }
    
    if (job.points.isEmpty()) {
        return false;
    }
    
    if (job.priority < 1 || job.priority > 5) {
        return false;
    }
    
    if (!job.deadline.isValid()) {
        return false;
    }
    
    return true;
}

void JobManager::updateJobStatus(const QString &jobId, const QString &status) {
    if (jobs.contains(jobId)) {
        jobs[jobId].status = status;
        emit jobUpdated(jobId);
    }
}

QVector<QPointF> JobManager::detectFiducials(const cv::Mat &image) {
    QVector<QPointF> fiducials;
    
    // Bildvorverarbeitung
    cv::Mat gray, binary;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    
    // Konturen finden
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Referenzmarken identifizieren
    for (const auto &contour : contours) {
        double area = cv::contourArea(contour);
        if (area > 100 && area < 1000) { // Größenfilter
            cv::Moments moments = cv::moments(contour);
            double x = moments.m10 / moments.m00;
            double y = moments.m01 / moments.m00;
            fiducials.append(QPointF(x, y));
        }
    }
    
    return fiducials;
}

bool JobManager::calculatePCBTransform(const PCBData &pcb, QTransform &transform) {
    if (pcb.fiducials.size() < 2) {
        return false;
    }
    
    // Referenzpunkte der Platine
    QPointF ref1 = pcb.fiducials[0];
    QPointF ref2 = pcb.fiducials[1];
    
    // Winkel berechnen
    double angle = std::atan2(ref2.y() - ref1.y(), ref2.x() - ref1.x());
    
    // Transformation erstellen
    transform.reset();
    transform.translate(ref1.x(), ref1.y());
    transform.rotate(angle * 180.0 / M_PI);
    transform.translate(-ref1.x(), -ref1.y());
    
    return true;
}

void JobManager::optimizePointSequence(QVector<SolderPoint> &points) {
    // Nearest-Neighbor-Algorithmus für die Optimierung der Reihenfolge
    if (points.size() < 2) return;
    
    QVector<SolderPoint> optimized;
    QVector<bool> visited(points.size(), false);
    optimized.append(points[0]);
    visited[0] = true;
    
    while (optimized.size() < points.size()) {
        int lastIndex = optimized.size() - 1;
        QVector3D lastPos = optimized[lastIndex].position;
        
        // Nächsten nächstgelegenen Punkt finden
        double minDist = std::numeric_limits<double>::max();
        int nextIndex = -1;
        
        for (int i = 0; i < points.size(); ++i) {
            if (!visited[i]) {
                double dist = (points[i].position - lastPos).length();
                if (dist < minDist) {
                    minDist = dist;
                    nextIndex = i;
                }
            }
        }
        
        if (nextIndex != -1) {
            optimized.append(points[nextIndex]);
            visited[nextIndex] = true;
        }
    }
    
    points = optimized;
}
