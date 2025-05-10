#include "networkserver.h"
#include <QDebug>

NetworkServer::NetworkServer(QObject *parent)
    : QObject(parent), server(new QTcpServer(this))
{
    connect(server, &QTcpServer::newConnection, this, &NetworkServer::onNewConnection);
}

bool NetworkServer::startServer(quint16 port)
{
    return server->listen(QHostAddress::Any, port);
}

void NetworkServer::stopServer()
{
    server->close();

    for (QTcpSocket *socket : clientSockets) {
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->disconnectFromHost();
            socket->waitForDisconnected(1000);
        }
        socket->deleteLater();
    }

    clientSockets.clear();
    activeClientSocket = nullptr;
}

void NetworkServer::pauseLoop()
{
    loopActive = false;
}
void NetworkServer::resumeLoop()
{
    loopActive = true;
}

void NetworkServer::onNewConnection()
{
    while (server->hasPendingConnections()) {
        QTcpSocket *newClient = server->nextPendingConnection();

        QString clientId = generateClientId();  // Unikalny identyfikator
        clientIds[newClient] = clientId;        // Przechowywanie ID dla danego klienta
        connect(newClient, &QTcpSocket::readyRead, this, &NetworkServer::onReadyRead);
        connect(newClient, &QTcpSocket::disconnected, this, &NetworkServer::onClientDisconnected);

        clientSockets.append(newClient);

        if (!activeClientSocket) {
            activeClientSocket = newClient;
        }

        QString ip = newClient->peerAddress().toString();
        if (newClient->peerAddress().protocol() == QAbstractSocket::IPv6Protocol &&
            ip.startsWith("::ffff:")) {
            ip = ip.mid(7);
        }
        emit clientConnected(clientId, ip); // Wyślij ID klienta do GUI
    }
}

void NetworkServer::onReadyRead()
{
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket || senderSocket != activeClientSocket)
        return;

    QByteArray buf = senderSocket->readAll();
    QDataStream in(&buf, QIODevice::ReadOnly);
    double val; in >> val;

    double out = obliczeniaSerwer(val);
    emit messageReceived(QString::number(out));

    QByteArray bout;
    QDataStream outStream(&bout, QIODevice::WriteOnly);
    outStream << out;
    senderSocket->write(bout);
}

void NetworkServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (!client) return;

    QString ip = socketToIpMap.value(client, "Nieznany IP");

    // Usuń klienta z listy i mapy
    clientSockets.removeAll(client);
    socketToIpMap.remove(client);

    // Jeśli rozłączył się aktywny klient — zmień na innego
    if (activeClientSocket == client) {
        activeClientSocket = clientSockets.isEmpty() ? nullptr : clientSockets.first();
    }

    client->deleteLater();
    emit clientDisconnected(ip);
}

double NetworkServer::obliczeniaSerwer(double input)
{
    double delta = -0.01 + QRandomGenerator::global()->generateDouble() * 0.02;
    return input += delta;
}

void NetworkServer::setActiveClient(int index)
{
    if (index >= 0 && index < clientSockets.size()) {
        activeClientSocket = clientSockets[index];
        qDebug() << "Wybrano aktywnego klienta:" << activeClientSocket->peerAddress().toString();
    }
}

QString NetworkServer::generateClientId()
{
    static int clientIdCounter = 1; // Unikalny licznik ID
    return QString("client_%1").arg(clientIdCounter++);
}

void NetworkServer::setActiveClientById(const QString &clientId)
{
    for (QTcpSocket *client : clientSockets) {
        if (clientIds[client] == clientId) {
            activeClientSocket = client;
            break;
        }
    }
}
