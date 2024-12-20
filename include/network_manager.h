#ifndef SOLDERROBOT_NETWORK_MANAGER_H
#define SOLDERROBOT_NETWORK_MANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QHttpServer>
#include <QSslCertificate>
#include <QSslKey>

struct RemoteClient {
    QString id;
    QString name;
    QString role;
    QDateTime connectedSince;
    bool isAuthorized;
};

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    
    // Web-Server
    bool startWebServer(quint16 port = 8080);
    void stopWebServer();
    
    // WebSocket-Server
    bool startWebSocketServer(quint16 port = 8081);
    void stopWebSocketServer();
    
    // Remote-Zugriff
    bool enableRemoteAccess(bool enable);
    QVector<RemoteClient> getConnectedClients() const;
    void disconnectClient(const QString &clientId);
    
    // Cloud-Integration
    bool connectToCloud(const QString &apiKey);
    void syncWithCloud();
    bool backupToCloud(const QString &data);
    QString restoreFromCloud();
    
    // Benachrichtigungen
    void sendNotification(const QString &message, const QString &severity);
    void configureEmailSettings(const QString &server, const QString &user, 
                              const QString &password);
    void configureSMSSettings(const QString &provider, const QString &apiKey);
    
signals:
    void clientConnected(const RemoteClient &client);
    void clientDisconnected(const QString &clientId);
    void dataReceived(const QString &clientId, const QByteArray &data);
    void cloudSyncCompleted();
    void cloudError(const QString &error);
    void notificationSent(const QString &to, const QString &message);
    void securityAlert(const QString &alert);

private:
    QHttpServer *webServer;
    QWebSocket *webSocket;
    QMap<QString, RemoteClient> clients;
    bool remoteAccessEnabled;
    
    // SSL/TLS
    QSslCertificate certificate;
    QSslKey privateKey;
    
    void setupSSL();
    void handleNewConnection();
    void handleWebSocketMessage(const QString &message);
    bool authenticateClient(const QString &credentials);
    void setupAPIEndpoints();
    void logNetworkActivity(const QString &activity);
};

#endif // SOLDERROBOT_NETWORK_MANAGER_H
