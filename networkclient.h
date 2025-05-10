#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QTcpSocket>
#include <QMessageBox>

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    void connectToServer(const QString &host, quint16 port);
    void sendDataToServer(double data);
    QTcpSocket* getSocket()  { return this->socket; }
    void setIsActive(bool isActive){ this->isActive = isActive; }

    bool getIsActive(){ return this->isActive; }
    bool isConnected() const;
    void disconnectFromServer();
public slots:
    void onReadyRead();

signals:
    void connected();
    void messageReceived(const QString &msg);
    void serverDisconnected();

private slots:
    void onConnected();

private:
    QTcpSocket *socket;
    double pendingValue = 0.0;
    bool hasPendingValue = false;
    bool loopActive = false;
    bool isActive = false;

    double obliczeniaKlient(double input);
};

#endif // NETWORKCLIENT_H
