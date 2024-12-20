#include "network_manager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QSslConfiguration>
#include <QSmtpClient>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , webServer(new QHttpServer(this))
    , webSocket(new QWebSocket)
    , remoteAccessEnabled(false)
{
    setupSSL();
}

bool NetworkManager::startWebServer(quint16 port) {
    // HTTPS-Server konfigurieren
    webServer->setServerConfiguration(certificate, privateKey);
    
    // API-Endpunkte einrichten
    setupAPIEndpoints();
    
    return webServer->listen(QHostAddress::Any, port);
}

void NetworkManager::stopWebServer() {
    webServer->close();
}

bool NetworkManager::startWebSocketServer(quint16 port) {
    // WebSocket-Server für Echtzeit-Updates
    connect(webSocket, &QWebSocket::connected, this, &NetworkManager::handleNewConnection);
    connect(webSocket, &QWebSocket::textMessageReceived,
            this, &NetworkManager::handleWebSocketMessage);
    
    return webSocket->listen(QHostAddress::Any, port);
}

void NetworkManager::stopWebSocketServer() {
    webSocket->close();
}

bool NetworkManager::enableRemoteAccess(bool enable) {
    remoteAccessEnabled = enable;
    
    if (enable) {
        // Sicherheitsüberprüfungen durchführen
        if (!certificate.isValid()) {
            emit securityAlert("Ungültiges SSL-Zertifikat");
            return false;
        }
    } else {
        // Alle bestehenden Verbindungen trennen
        for (const auto &client : clients.keys()) {
            disconnectClient(client);
        }
    }
    
    return true;
}

QVector<RemoteClient> NetworkManager::getConnectedClients() const {
    return QVector<RemoteClient>(clients.values());
}

void NetworkManager::disconnectClient(const QString &clientId) {
    if (clients.contains(clientId)) {
        emit clientDisconnected(clientId);
        clients.remove(clientId);
    }
}

bool NetworkManager::connectToCloud(const QString &apiKey) {
    // Cloud-Verbindung aufbauen
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("https://cloud-api.example.com/connect"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
    
    QJsonObject payload;
    payload["device_id"] = "solder_robot_1";
    payload["version"] = "1.0";
    
    QNetworkReply *reply = manager->post(request, QJsonDocument(payload).toJson());
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit cloudSyncCompleted();
        } else {
            emit cloudError(reply->errorString());
        }
        reply->deleteLater();
    });
    
    return true;
}

void NetworkManager::syncWithCloud() {
    // Daten mit Cloud synchronisieren
    if (!remoteAccessEnabled) {
        emit cloudError("Remote-Zugriff deaktiviert");
        return;
    }
    
    // Synchronisationsdaten sammeln
    QJsonObject syncData;
    syncData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    syncData["status"] = "online";
    
    // Sync durchführen
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("https://cloud-api.example.com/sync"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply *reply = manager->post(request, QJsonDocument(syncData).toJson());
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit cloudSyncCompleted();
        } else {
            emit cloudError(reply->errorString());
        }
        reply->deleteLater();
    });
}

bool NetworkManager::backupToCloud(const QString &data) {
    if (!remoteAccessEnabled) {
        return false;
    }
    
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("https://cloud-api.example.com/backup"));
    
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QHttpPart dataPart;
    dataPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    dataPart.setBody(data.toUtf8());
    
    multiPart->append(dataPart);
    
    QNetworkReply *reply = manager->post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit cloudSyncCompleted();
        } else {
            emit cloudError(reply->errorString());
        }
        reply->deleteLater();
    });
    
    return true;
}

QString NetworkManager::restoreFromCloud() {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl("https://cloud-api.example.com/restore"));
    
    QNetworkReply *reply = manager->get(request);
    
    // Synchroner Aufruf für die Wiederherstellung
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        return QString::fromUtf8(reply->readAll());
    } else {
        emit cloudError(reply->errorString());
        return QString();
    }
}

void NetworkManager::sendNotification(const QString &message, const QString &severity) {
    // E-Mail-Benachrichtigung
    QSmtpClient smtp;
    smtp.setHost("smtp.example.com");
    smtp.setPort(587);
    smtp.setConnectionType(QSmtpClient::TlsConnection);
    
    QEmailAddress sender("robot@example.com", "Solder Robot");
    QEmailAddress recipient("admin@example.com", "Administrator");
    
    QEmailMessage email;
    email.setSender(sender);
    email.addRecipient(recipient);
    email.setSubject(QString("[%1] Solder Robot Notification").arg(severity));
    email.setBody(message);
    
    if (!smtp.sendMail(email)) {
        emit securityAlert("E-Mail-Benachrichtigung fehlgeschlagen");
    }
}

void NetworkManager::setupSSL() {
    // SSL-Konfiguration
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_3);
    
    // Zertifikat laden
    QFile certFile("server.crt");
    if (certFile.open(QIODevice::ReadOnly)) {
        certificate = QSslCertificate(&certFile, QSsl::Pem);
        certFile.close();
    }
    
    // Privaten Schlüssel laden
    QFile keyFile("server.key");
    if (keyFile.open(QIODevice::ReadOnly)) {
        privateKey = QSslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
        keyFile.close();
    }
}

void NetworkManager::handleNewConnection() {
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) return;
    
    RemoteClient client;
    client.id = QUuid::createUuid().toString();
    client.connectedSince = QDateTime::currentDateTime();
    
    // Authentifizierung überprüfen
    QString authHeader = socket->request().rawHeader("Authorization");
    client.isAuthorized = authenticateClient(authHeader);
    
    if (client.isAuthorized) {
        clients[client.id] = client;
        emit clientConnected(client);
    } else {
        socket->close();
        emit securityAlert("Nicht autorisierter Verbindungsversuch");
    }
}

void NetworkManager::handleWebSocketMessage(const QString &message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject obj = doc.object();
    
    QString clientId = obj["client_id"].toString();
    if (!clients.contains(clientId)) {
        emit securityAlert("Nachricht von unbekanntem Client");
        return;
    }
    
    // Nachricht verarbeiten
    emit dataReceived(clientId, message.toUtf8());
}

bool NetworkManager::authenticateClient(const QString &credentials) {
    // Basic Auth Header auswerten
    if (credentials.startsWith("Basic ")) {
        QString decoded = QByteArray::fromBase64(credentials.mid(6).toUtf8());
        QStringList parts = decoded.split(':');
        if (parts.size() == 2) {
            // Hier würde die tatsächliche Authentifizierung stattfinden
            return true;
        }
    }
    return false;
}

void NetworkManager::setupAPIEndpoints() {
    // REST API Endpunkte
    webServer->route("/api/status", [this](const QHttpServerRequest &request) {
        QJsonObject status;
        status["clients"] = clients.size();
        status["uptime"] = QDateTime::currentDateTime().toString();
        return QHttpServerResponse(status);
    });
    
    webServer->route("/api/control", [this](const QHttpServerRequest &request) {
        if (!authenticateClient(request.value("Authorization"))) {
            return QHttpServerResponse(QHttpServerResponse::StatusCode::Unauthorized);
        }
        return QHttpServerResponse(QHttpServerResponse::StatusCode::Ok);
    });
}

void NetworkManager::logNetworkActivity(const QString &activity) {
    qDebug() << "Network Activity:" << activity;
    // Hier könnte eine ausführlichere Protokollierung erfolgen
}
