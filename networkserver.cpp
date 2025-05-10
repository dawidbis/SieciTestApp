#include "networkserver.h"
#include <QDebug>

NetworkServer::NetworkServer(QObject *parent)
    : QObject(parent), server(new QTcpServer(this)), clientSocket(nullptr)
{
    connect(server, &QTcpServer::newConnection, this, &NetworkServer::onNewConnection);
}

bool NetworkServer::startServer(quint16 port)
{
    return server->listen(QHostAddress::Any, port);
}

void NetworkServer::stopServer()
{
    // Przestań nasłuchiwać nowych połączeń
    server->close();

    // Jeśli istnieje połączenie z klientem, rozłącz je
    if (clientSocket) {
        clientSocket->disconnectFromHost();
        // Poczekaj maksymalnie 1s na rozłączenie
        if (!clientSocket->waitForDisconnected(1000)) {
            qDebug() << "NetworkServer::stopServer - klient nie rozłączył się na czas";
        }
        // Usuń socket klienta po zakończeniu eventów
        clientSocket->deleteLater();
        clientSocket = nullptr;
    }
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
    clientSocket = server->nextPendingConnection();

    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkServer::onReadyRead);

    connect(clientSocket, &QTcpSocket::disconnected, this, [this]() {
        qDebug() << "NetworkServer: klient się rozłączył";
        emit clientDisconnected();
    });

    QString ip = clientSocket->peerAddress().toString();
    if (clientSocket->peerAddress().protocol() == QAbstractSocket::IPv6Protocol &&
        ip.startsWith("::ffff:")) {
        ip = ip.mid(7);  // usuń "::ffff:" z początku
    }
    emit clientConnected(ip);
}

void NetworkServer::onReadyRead()
{
    QByteArray buf = clientSocket->readAll();
    QDataStream in(&buf, QIODevice::ReadOnly);
    double val; in >> val;

    double out = obliczeniaSerwer(val);
    emit messageReceived(QString::number(out));

    QByteArray bout;
    QDataStream outStream(&bout, QIODevice::WriteOnly);
    outStream << out;
    clientSocket->write(bout);
}

double NetworkServer::obliczeniaSerwer(double input)
{
    double delta = -0.01 + QRandomGenerator::global()->generateDouble() * 0.02;
    return input += delta;
}
