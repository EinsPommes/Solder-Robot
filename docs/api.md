# API-Referenz - Solder-Robot

## 1. REST API

### 1.1 Authentifizierung
```http
POST /api/auth/login
Content-Type: application/json

{
    "username": "string",
    "password": "string"
}
```

### 1.2 Jobs

#### Job erstellen
```http
POST /api/jobs
Content-Type: application/json

{
    "name": "string",
    "points": [
        {
            "position": {
                "x": 0,
                "y": 0,
                "z": 0
            },
            "temperature": 350,
            "dwellTime": 1000
        }
    ]
}
```

#### Job abrufen
```http
GET /api/jobs/{jobId}
```

#### Jobs auflisten
```http
GET /api/jobs?status=waiting&limit=10
```

#### Job aktualisieren
```http
PUT /api/jobs/{jobId}
Content-Type: application/json

{
    "status": "completed"
}
```

### 1.3 Prozesssteuerung

#### Prozess starten
```http
POST /api/process/start/{jobId}
```

#### Prozess stoppen
```http
POST /api/process/stop/{jobId}
```

#### Status abrufen
```http
GET /api/process/status/{jobId}
```

## 2. WebSocket API

### 2.1 Verbindung
```javascript
const ws = new WebSocket('ws://localhost:8080/ws');
```

### 2.2 Events

#### Status-Updates
```javascript
// Empfangen
{
    "type": "status",
    "data": {
        "jobId": "string",
        "status": "running",
        "progress": 75,
        "temperature": 350
    }
}
```

#### Fehler-Events
```javascript
// Empfangen
{
    "type": "error",
    "data": {
        "code": "E001",
        "message": "Temperature too high"
    }
}
```

## 3. C++ API

### 3.1 Job Management
```cpp
class JobManager {
public:
    // Jobs verwalten
    QString createJob(const SolderJob &job);
    bool updateJob(const QString &jobId, const SolderJob &job);
    bool deleteJob(const QString &jobId);
    SolderJob getJob(const QString &jobId) const;
    QVector<SolderJob> getAllJobs() const;
    
    // Prozesssteuerung
    bool startJob(const QString &jobId);
    bool pauseJob(const QString &jobId);
    bool resumeJob(const QString &jobId);
    bool abortJob(const QString &jobId);
};
```

### 3.2 Motion Control
```cpp
class MotionController {
public:
    // Bewegungssteuerung
    bool moveToPosition(const QVector3D &position);
    bool setSpeed(double speed);
    bool setAcceleration(double acceleration);
    
    // Status
    QVector3D getCurrentPosition() const;
    bool isMoving() const;
    
    // Kalibrierung
    bool calibrate();
    bool homeAxes();
};
```

### 3.3 Temperature Control
```cpp
class TemperatureControl {
public:
    // Temperatursteuerung
    bool setTemperature(double temperature);
    bool enableHeating(bool enable);
    
    // Monitoring
    double getCurrentTemperature() const;
    bool isHeating() const;
    
    // Sicherheit
    void emergencyShutdown();
};
```

### 3.4 Vision System
```cpp
class VisionSystem {
public:
    // Bildaufnahme
    cv::Mat captureImage();
    bool startLiveView();
    bool stopLiveView();
    
    // Analyse
    bool detectSolderPoints(cv::Mat &image);
    bool analyzeSolderJoint(const cv::Mat &image);
    
    // Kalibrierung
    bool calibrateCamera();
    bool setCameraParameters(const CameraParameters &params);
};
```

## 4. Event System

### 4.1 Signale
```cpp
// Job Events
void jobCreated(const QString &jobId);
void jobUpdated(const QString &jobId);
void jobStarted(const QString &jobId);
void jobCompleted(const QString &jobId);
void jobError(const QString &jobId, const QString &error);

// Process Events
void progressUpdated(const QString &jobId, int current, int total);
void temperatureChanged(double temperature);
void positionChanged(const QVector3D &position);

// System Events
void systemError(const QString &error);
void systemWarning(const QString &warning);
void systemStatus(const QString &status);
```

### 4.2 Event Handler
```cpp
class EventHandler {
public:
    void subscribe(const QString &event, std::function<void(const QJsonObject&)> callback);
    void unsubscribe(const QString &event);
    void emit(const QString &event, const QJsonObject &data);
};
```

## 5. Datenbank-Schema

### 5.1 Jobs
```sql
CREATE TABLE jobs (
    id TEXT PRIMARY KEY,
    name TEXT,
    created DATETIME,
    status TEXT,
    priority INTEGER,
    data JSON
);
```

### 5.2 Process Data
```sql
CREATE TABLE process_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    job_id TEXT,
    timestamp DATETIME,
    temperature REAL,
    position_x REAL,
    position_y REAL,
    position_z REAL,
    solder_flow REAL
);
```

## 6. Konfiguration

### 6.1 System-Konfiguration
```json
{
    "hardware": {
        "motion": {
            "maxSpeed": 100,
            "maxAcceleration": 1000,
            "stepResolution": 0.1
        },
        "temperature": {
            "maxTemp": 450,
            "minTemp": 200,
            "pidParams": {
                "kp": 1.0,
                "ki": 0.1,
                "kd": 0.01
            }
        }
    },
    "network": {
        "port": 8080,
        "maxConnections": 10,
        "timeout": 30000
    }
}
```

### 6.2 Sicherheits-Konfiguration
```json
{
    "security": {
        "authentication": true,
        "tokenExpiration": 3600,
        "maxLoginAttempts": 3
    },
    "limits": {
        "maxTemperature": 450,
        "maxSpeed": 100,
        "safeZone": {
            "minX": 0,
            "maxX": 300,
            "minY": 0,
            "maxY": 200,
            "minZ": 0,
            "maxZ": 100
        }
    }
}
```
