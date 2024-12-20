#ifndef SOLDERROBOT_DATA_LOGGER_H
#define SOLDERROBOT_DATA_LOGGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QDateTime>
#include <QJsonDocument>

struct LogEntry {
    QDateTime timestamp;
    QString category;
    QString event;
    QJsonDocument data;
    QString severity;
    QString source;
};

struct ProcessData {
    QDateTime timestamp;
    double temperature;
    QVector3D position;
    double solderFlow;
    double energy;
    QString programName;
    int cycleCount;
};

class DataLogger : public QObject {
    Q_OBJECT

public:
    explicit DataLogger(QObject *parent = nullptr);
    
    // Datenbankverbindung
    bool initialize(const QString &dbPath);
    void close();
    
    // Protokollierung
    void logEvent(const LogEntry &entry);
    void logProcessData(const ProcessData &data);
    void logError(const QString &source, const QString &message);
    void logWarning(const QString &source, const QString &message);
    
    // Datenabfrage
    QVector<LogEntry> getEvents(const QDateTime &start, const QDateTime &end,
                               const QString &category = QString()) const;
    QVector<ProcessData> getProcessData(const QDateTime &start,
                                      const QDateTime &end) const;
    
    // Statistiken
    int getErrorCount(const QString &source) const;
    double getAverageTemperature(const QString &program) const;
    int getTotalCycles() const;
    QJsonDocument getProcessStatistics(const QString &program) const;
    
    // Datenexport
    bool exportToCSV(const QString &filename, const QDateTime &start,
                    const QDateTime &end);
    bool exportToJSON(const QString &filename, const QDateTime &start,
                     const QDateTime &end);
    
signals:
    void logEntryAdded(const LogEntry &entry);
    void processDataRecorded(const ProcessData &data);
    void databaseError(const QString &error);
    void exportCompleted(const QString &filename);

private:
    QSqlDatabase db;
    QString databasePath;
    
    bool createTables();
    void cleanupOldData(int daysToKeep);
    void optimizeDatabase();
    void backupDatabase();
    QString formatLogEntry(const LogEntry &entry) const;
    void validateData(const ProcessData &data);
};

#endif // SOLDERROBOT_DATA_LOGGER_H
