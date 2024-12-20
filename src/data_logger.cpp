#include "data_logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QDebug>

DataLogger::DataLogger(QObject *parent)
    : QObject(parent)
{
}

bool DataLogger::initialize(const QString &dbPath) {
    databasePath = dbPath;
    
    // Datenbankverbindung aufbauen
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        emit databaseError("Datenbank konnte nicht geöffnet werden: " + 
                         db.lastError().text());
        return false;
    }
    
    // Tabellen erstellen
    if (!createTables()) {
        return false;
    }
    
    // Alte Daten bereinigen
    cleanupOldData(30); // 30 Tage aufbewahren
    
    // Datenbank optimieren
    optimizeDatabase();
    
    return true;
}

void DataLogger::close() {
    if (db.isOpen()) {
        backupDatabase();
        db.close();
    }
}

void DataLogger::logEvent(const LogEntry &entry) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO events (timestamp, category, event, data, severity, source) "
                 "VALUES (?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(entry.timestamp);
    query.addBindValue(entry.category);
    query.addBindValue(entry.event);
    query.addBindValue(entry.data.toJson());
    query.addBindValue(entry.severity);
    query.addBindValue(entry.source);
    
    if (!query.exec()) {
        emit databaseError("Fehler beim Protokollieren des Ereignisses: " + 
                         query.lastError().text());
        return;
    }
    
    emit logEntryAdded(entry);
}

void DataLogger::logProcessData(const ProcessData &data) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO process_data (timestamp, temperature, position_x, "
                 "position_y, position_z, solder_flow, energy, program_name, cycle_count) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(data.timestamp);
    query.addBindValue(data.temperature);
    query.addBindValue(data.position.x());
    query.addBindValue(data.position.y());
    query.addBindValue(data.position.z());
    query.addBindValue(data.solderFlow);
    query.addBindValue(data.energy);
    query.addBindValue(data.programName);
    query.addBindValue(data.cycleCount);
    
    if (!query.exec()) {
        emit databaseError("Fehler beim Protokollieren der Prozessdaten: " + 
                         query.lastError().text());
        return;
    }
    
    emit processDataRecorded(data);
}

void DataLogger::logError(const QString &source, const QString &message) {
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.category = "error";
    entry.event = message;
    entry.severity = "error";
    entry.source = source;
    
    logEvent(entry);
}

void DataLogger::logWarning(const QString &source, const QString &message) {
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.category = "warning";
    entry.event = message;
    entry.severity = "warning";
    entry.source = source;
    
    logEvent(entry);
}

QVector<LogEntry> DataLogger::getEvents(const QDateTime &start, const QDateTime &end,
                                      const QString &category) const {
    QVector<LogEntry> entries;
    QSqlQuery query(db);
    
    QString sql = "SELECT * FROM events WHERE timestamp BETWEEN ? AND ?";
    if (!category.isEmpty()) {
        sql += " AND category = ?";
    }
    sql += " ORDER BY timestamp DESC";
    
    query.prepare(sql);
    query.addBindValue(start);
    query.addBindValue(end);
    if (!category.isEmpty()) {
        query.addBindValue(category);
    }
    
    if (query.exec()) {
        while (query.next()) {
            LogEntry entry;
            entry.timestamp = query.value("timestamp").toDateTime();
            entry.category = query.value("category").toString();
            entry.event = query.value("event").toString();
            entry.data = QJsonDocument::fromJson(query.value("data").toByteArray());
            entry.severity = query.value("severity").toString();
            entry.source = query.value("source").toString();
            entries.append(entry);
        }
    }
    
    return entries;
}

QVector<ProcessData> DataLogger::getProcessData(const QDateTime &start,
                                              const QDateTime &end) const {
    QVector<ProcessData> dataPoints;
    QSqlQuery query(db);
    
    query.prepare("SELECT * FROM process_data WHERE timestamp BETWEEN ? AND ? "
                 "ORDER BY timestamp ASC");
    query.addBindValue(start);
    query.addBindValue(end);
    
    if (query.exec()) {
        while (query.next()) {
            ProcessData data;
            data.timestamp = query.value("timestamp").toDateTime();
            data.temperature = query.value("temperature").toDouble();
            data.position = QVector3D(
                query.value("position_x").toFloat(),
                query.value("position_y").toFloat(),
                query.value("position_z").toFloat()
            );
            data.solderFlow = query.value("solder_flow").toDouble();
            data.energy = query.value("energy").toDouble();
            data.programName = query.value("program_name").toString();
            data.cycleCount = query.value("cycle_count").toInt();
            dataPoints.append(data);
        }
    }
    
    return dataPoints;
}

int DataLogger::getErrorCount(const QString &source) const {
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM events WHERE source = ? AND severity = 'error'");
    query.addBindValue(source);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

double DataLogger::getAverageTemperature(const QString &program) const {
    QSqlQuery query(db);
    query.prepare("SELECT AVG(temperature) FROM process_data WHERE program_name = ?");
    query.addBindValue(program);
    
    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}

int DataLogger::getTotalCycles() const {
    QSqlQuery query(db);
    if (query.exec("SELECT MAX(cycle_count) FROM process_data") && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QJsonDocument DataLogger::getProcessStatistics(const QString &program) const {
    QJsonObject stats;
    QSqlQuery query(db);
    
    // Temperaturstatistiken
    query.prepare("SELECT MIN(temperature) as min_temp, "
                 "MAX(temperature) as max_temp, "
                 "AVG(temperature) as avg_temp "
                 "FROM process_data WHERE program_name = ?");
    query.addBindValue(program);
    
    if (query.exec() && query.next()) {
        QJsonObject tempStats;
        tempStats["min"] = query.value("min_temp").toDouble();
        tempStats["max"] = query.value("max_temp").toDouble();
        tempStats["avg"] = query.value("avg_temp").toDouble();
        stats["temperature"] = tempStats;
    }
    
    // Energiestatistiken
    query.prepare("SELECT SUM(energy) as total_energy, "
                 "AVG(energy) as avg_energy "
                 "FROM process_data WHERE program_name = ?");
    query.addBindValue(program);
    
    if (query.exec() && query.next()) {
        QJsonObject energyStats;
        energyStats["total"] = query.value("total_energy").toDouble();
        energyStats["average"] = query.value("avg_energy").toDouble();
        stats["energy"] = energyStats;
    }
    
    return QJsonDocument(stats);
}

bool DataLogger::exportToCSV(const QString &filename, const QDateTime &start,
                           const QDateTime &end) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Header schreiben
    out << "Timestamp,Temperature,Position_X,Position_Y,Position_Z,"
        << "SolderFlow,Energy,Program,CycleCount\n";
    
    // Daten exportieren
    QVector<ProcessData> data = getProcessData(start, end);
    for (const auto &point : data) {
        out << point.timestamp.toString(Qt::ISODate) << ","
            << point.temperature << ","
            << point.position.x() << ","
            << point.position.y() << ","
            << point.position.z() << ","
            << point.solderFlow << ","
            << point.energy << ","
            << point.programName << ","
            << point.cycleCount << "\n";
    }
    
    file.close();
    emit exportCompleted(filename);
    return true;
}

bool DataLogger::exportToJSON(const QString &filename, const QDateTime &start,
                            const QDateTime &end) {
    QJsonArray dataArray;
    
    // Prozessdaten exportieren
    QVector<ProcessData> data = getProcessData(start, end);
    for (const auto &point : data) {
        QJsonObject dataPoint;
        dataPoint["timestamp"] = point.timestamp.toString(Qt::ISODate);
        dataPoint["temperature"] = point.temperature;
        dataPoint["position"] = QJsonObject{
            {"x", point.position.x()},
            {"y", point.position.y()},
            {"z", point.position.z()}
        };
        dataPoint["solder_flow"] = point.solderFlow;
        dataPoint["energy"] = point.energy;
        dataPoint["program"] = point.programName;
        dataPoint["cycle_count"] = point.cycleCount;
        
        dataArray.append(dataPoint);
    }
    
    QJsonDocument doc(dataArray);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    emit exportCompleted(filename);
    return true;
}

bool DataLogger::createTables() {
    QSqlQuery query(db);
    
    // Ereignistabelle
    if (!query.exec("CREATE TABLE IF NOT EXISTS events ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "timestamp DATETIME,"
                   "category TEXT,"
                   "event TEXT,"
                   "data TEXT,"
                   "severity TEXT,"
                   "source TEXT)")) {
        emit databaseError("Fehler beim Erstellen der Ereignistabelle: " + 
                         query.lastError().text());
        return false;
    }
    
    // Prozessdatentabelle
    if (!query.exec("CREATE TABLE IF NOT EXISTS process_data ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "timestamp DATETIME,"
                   "temperature REAL,"
                   "position_x REAL,"
                   "position_y REAL,"
                   "position_z REAL,"
                   "solder_flow REAL,"
                   "energy REAL,"
                   "program_name TEXT,"
                   "cycle_count INTEGER)")) {
        emit databaseError("Fehler beim Erstellen der Prozessdatentabelle: " + 
                         query.lastError().text());
        return false;
    }
    
    return true;
}

void DataLogger::cleanupOldData(int daysToKeep) {
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-daysToKeep);
    
    QSqlQuery query(db);
    query.prepare("DELETE FROM events WHERE timestamp < ?");
    query.addBindValue(cutoff);
    query.exec();
    
    query.prepare("DELETE FROM process_data WHERE timestamp < ?");
    query.addBindValue(cutoff);
    query.exec();
}

void DataLogger::optimizeDatabase() {
    QSqlQuery query(db);
    query.exec("VACUUM");
    query.exec("ANALYZE");
}

void DataLogger::backupDatabase() {
    QString backupPath = databasePath + ".backup";
    QFile::copy(databasePath, backupPath);
}

QString DataLogger::formatLogEntry(const LogEntry &entry) const {
    return QString("[%1] %2 - %3: %4")
        .arg(entry.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(entry.severity.toUpper())
        .arg(entry.source)
        .arg(entry.event);
}

void DataLogger::validateData(const ProcessData &data) {
    // Datenvalidierung
    if (data.temperature < 0 || data.temperature > 450) {
        logWarning("DataLogger", "Ungültige Temperatur: " + 
                  QString::number(data.temperature));
    }
    
    if (data.solderFlow < 0) {
        logWarning("DataLogger", "Ungültiger Lötdrahtfluss: " + 
                  QString::number(data.solderFlow));
    }
    
    if (data.energy < 0) {
        logWarning("DataLogger", "Ungültiger Energiewert: " + 
                  QString::number(data.energy));
    }
}
