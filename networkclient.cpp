#include "networkclient.h"
#include "qtimer.h"

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent), socket(new QTcpSocket(this))
{
    // połączenie nawiązane
    connect(socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    // odczyt danych
    connect(socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
}

void NetworkClient::connectToServer(const QString &host, quint16 port)
{
    socket->connectToHost(host, port);
}

void NetworkClient::onConnected()
{
    emit connected();
}

void NetworkClient::onReadyRead()
{
    QByteArray buf = socket->readAll();
    QDataStream in(&buf, QIODevice::ReadOnly);
    double val; in >> val;

    // pokaż odpowiedź w GUI
    emit messageReceived(QString::number(val));

    // jeżeli pętla aktywna, wyślij kolejną iterację
    if (loopActive) {
        double next = obliczeniaKlient(val);
        sendDataToServer(next);
    }
}

double NetworkClient::obliczeniaKlient(double input)
{
    return (input += 0.01);
}

void NetworkClient::sendDataToServer(double data)
{
    if (socket->state() != QTcpSocket::ConnectedState)
        return;

    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    out << data;
    socket->write(buf);
}

bool NetworkClient::isConnected() const {
    return socket && socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::disconnectFromServer() {
    if (socket) {
        socket->disconnectFromHost();
    }
}

void NetworkClient::pauseLoop()
{
    loopActive = false;
}

void NetworkClient::resumeLoop()
{
     loopActive = true;
}
