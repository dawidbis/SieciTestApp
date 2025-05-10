#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QRandomGenerator>

class NetworkServer : public QObject
{
    Q_OBJECT

public:
    explicit NetworkServer(QObject *parent = nullptr);
    bool startServer(quint16 port);

    void stopServer();
    bool isConnected() const;

    void pauseLoop();
    void resumeLoop();
    void close();
signals:
    void messageReceived(const QString &message);
    void clientConnected(const QString &address);
    void clientDisconnected();

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    QTcpServer *server;
    QTcpSocket *clientSocket;

    bool loopActive = false;

    double obliczeniaSerwer(double input);
};

#endif // NETWORKSERVER_H
