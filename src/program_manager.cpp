#include "program_manager.h"
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ProgramManager::ProgramManager(QObject *parent)
    : QObject(parent)
    , programDirectory(QDir::current().filePath("solder_programs"))
    , isRecording(false)
    , isProgramRunning(false)
    , isPaused(false)
{
    // Programmverzeichnis erstellen, falls es nicht existiert
    QDir().mkpath(programDirectory);
}

bool ProgramManager::saveProgram(const SolderProgram &program) {
    if (!validateProgram(program)) {
        return false;
    }

    QFile file(generateProgramPath(program.name));
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Fehler beim Öffnen der Programmdatei:" << file.errorString();
        return false;
    }

    QJsonObject programObject;
    programObject["name"] = program.name;
    programObject["description"] = program.description;
    programObject["creator"] = program.creator;
    programObject["created"] = program.created.toString(Qt::ISODate);
    programObject["modified"] = program.modified.toString(Qt::ISODate);

    QJsonArray pointsArray;
    for (const auto &point : program.points) {
        QJsonObject pointObject;
        pointObject["x"] = point.x;
        pointObject["y"] = point.y;
        pointObject["z"] = point.z;
        pointObject["temperature"] = point.temperature;
        pointObject["dwellTime"] = point.dwellTime;
        pointObject["type"] = point.type;
        pointsArray.append(pointObject);
    }
    programObject["points"] = pointsArray;
    programObject["settings"] = QJsonDocument::fromJson(program.settings.toJson()).object();

    QJsonDocument doc(programObject);
    file.write(doc.toJson());
    return true;
}

bool ProgramManager::loadProgram(const QString &name, SolderProgram &program) {
    QFile file(generateProgramPath(name));
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Fehler beim Öffnen der Programmdatei:" << file.errorString();
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject programObject = doc.object();

    program.name = programObject["name"].toString();
    program.description = programObject["description"].toString();
    program.creator = programObject["creator"].toString();
    program.created = QDateTime::fromString(programObject["created"].toString(), Qt::ISODate);
    program.modified = QDateTime::fromString(programObject["modified"].toString(), Qt::ISODate);

    QJsonArray pointsArray = programObject["points"].toArray();
    program.points.clear();
    for (const auto &pointValue : pointsArray) {
        QJsonObject pointObject = pointValue.toObject();
        SolderPoint point;
        point.x = pointObject["x"].toDouble();
        point.y = pointObject["y"].toDouble();
        point.z = pointObject["z"].toDouble();
        point.temperature = pointObject["temperature"].toDouble();
        point.dwellTime = pointObject["dwellTime"].toInt();
        point.type = pointObject["type"].toString();
        program.points.append(point);
    }

    program.settings = QJsonDocument(programObject["settings"].toObject());
    return true;
}

QStringList ProgramManager::listPrograms() const {
    QDir dir(programDirectory);
    return dir.entryList(QStringList() << "*.json", QDir::Files);
}

bool ProgramManager::deleteProgram(const QString &name) {
    return QFile::remove(generateProgramPath(name));
}

void ProgramManager::startRecording() {
    if (!isRecording) {
        recordedPoints.clear();
        isRecording = true;
        emit recordingStarted();
    }
}

void ProgramManager::stopRecording() {
    if (isRecording) {
        isRecording = false;
        emit recordingStopped();
    }
}

void ProgramManager::addPoint(const SolderPoint &point) {
    if (isRecording) {
        recordedPoints.append(point);
        emit pointAdded(point);
    }
}

void ProgramManager::clearRecording() {
    recordedPoints.clear();
}

void ProgramManager::startProgram(const QString &name) {
    if (isProgramRunning) {
        return;
    }

    if (loadProgram(name, currentProgram)) {
        isProgramRunning = true;
        isPaused = false;
        emit programStarted(name);
        
        // Programm ausführen...
        for (int i = 0; i < currentProgram.points.size(); ++i) {
            if (!isProgramRunning || isPaused) {
                break;
            }
            
            emit progressUpdated(i + 1, currentProgram.points.size());
            // Hier würde die tatsächliche Ausführung der Lötpunkte erfolgen
        }

        if (isProgramRunning && !isPaused) {
            emit programCompleted();
        }
    } else {
        emit programError("Programm konnte nicht geladen werden");
    }
}

void ProgramManager::pauseProgram() {
    if (isProgramRunning && !isPaused) {
        isPaused = true;
        emit programPaused();
    }
}

void ProgramManager::resumeProgram() {
    if (isProgramRunning && isPaused) {
        isPaused = false;
        emit programResumed();
    }
}

void ProgramManager::stopProgram() {
    if (isProgramRunning) {
        isProgramRunning = false;
        isPaused = false;
        emit programStopped();
    }
}

bool ProgramManager::validateProgram(const SolderProgram &program) {
    if (program.name.isEmpty()) {
        return false;
    }
    
    if (program.points.isEmpty()) {
        return false;
    }
    
    for (const auto &point : program.points) {
        if (point.temperature < 0 || point.temperature > 450) {
            return false;
        }
        if (point.dwellTime < 0) {
            return false;
        }
    }
    
    return true;
}

QString ProgramManager::generateProgramPath(const QString &name) const {
    return QDir(programDirectory).filePath(name + ".json");
}
