#ifndef SOLDERROBOT_PROGRAM_MANAGER_H
#define SOLDERROBOT_PROGRAM_MANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonDocument>

struct SolderPoint {
    double x, y, z;
    double temperature;
    int dwellTime;  // Verweilzeit in ms
    QString type;   // "through-hole", "smd", etc.
};

struct SolderProgram {
    QString name;
    QString description;
    QVector<SolderPoint> points;
    QJsonDocument settings;
    QString creator;
    QDateTime created;
    QDateTime modified;
};

class ProgramManager : public QObject {
    Q_OBJECT

public:
    explicit ProgramManager(QObject *parent = nullptr);
    
    // Programm-Verwaltung
    bool saveProgram(const SolderProgram &program);
    bool loadProgram(const QString &name, SolderProgram &program);
    QStringList listPrograms() const;
    bool deleteProgram(const QString &name);
    
    // Teach-In-Modus
    void startRecording();
    void stopRecording();
    void addPoint(const SolderPoint &point);
    void clearRecording();
    
    // Programm-Ausführung
    void startProgram(const QString &name);
    void pauseProgram();
    void resumeProgram();
    void stopProgram();
    
signals:
    void recordingStarted();
    void recordingStopped();
    void pointAdded(const SolderPoint &point);
    void programStarted(const QString &name);
    void programPaused();
    void programResumed();
    void programStopped();
    void programCompleted();
    void programError(const QString &error);
    void progressUpdated(int current, int total);

private:
    QString programDirectory;
    bool isRecording;
    bool isProgramRunning;
    bool isPaused;
    SolderProgram currentProgram;
    QVector<SolderPoint> recordedPoints;
    
    bool validateProgram(const SolderProgram &program);
    QString generateProgramPath(const QString &name) const;
};

#endif // SOLDERROBOT_PROGRAM_MANAGER_H
