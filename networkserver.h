#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QRandomGenerator>
#include <QMap>

class NetworkServer : public QObject
{
    Q_OBJECT

public:
    explicit NetworkServer(QObject *parent = nullptr);
    bool startServer(quint16 port);
    void stopServer();
    void pauseLoop();
    void resumeLoop();
    void setActiveClient(int index);
    void setActiveClientById(const QString &clientId);
    QTcpSocket* getActiveClient() const { return activeClientSocket; }
    QString getActiveClientIp() const;

signals:
    void messageReceived(const QString &message);
    void clientConnected(const QString &clientId, const QString &ip);
    void clientDisconnected(const QString &ip);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *server;
    QList<QTcpSocket*> clientSockets;
    QMap<QTcpSocket*, QString> clientIds;  // Mapa socketów i ich ID
    QMap<QTcpSocket*, QString> socketToIpMap;  // Mapa socketów i ich adresów IP
    QTcpSocket *activeClientSocket = nullptr;
    bool loopActive = false;

    double obliczeniaSerwer(double input);
    QString generateClientId();
};

#endif // NETWORKSERVER_H
