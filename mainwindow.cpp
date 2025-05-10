#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->label->hide();
    ui->dsb_Dane->hide();
    ui->lbl_Opis->hide();
    ui->lbl_WynikDane->hide();
    ui->pushButton->hide();
    ui->btnStartLoop->hide();
    ui->lbl_klie->hide();
    ui->rbtn_taktowanie1->hide();
    ui->rbtn_taktowanie2->hide();

    clientListModel = new QStandardItemModel(this);
    ui->listView->setModel(clientListModel);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex &) {
                if (current.isValid()) {
                    setActiveClient(current.row());
                }
            });

    // ukryj na starcie
    ui->listView->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_TrybSieciowy_clicked()
{

    RoleDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        isServer = dialog.isServerSelected();
        selectedIp = dialog.getAddress();
        selectedPort = dialog.getPort();
        trybSieciowyUstawiony = true;

        // Opcjonalnie pokaż info
        qDebug() << "Tryb ustawiony: " << (isServer ? "Serwer" : "Klient")
                 << "IP:" << selectedIp << "Port:" << selectedPort;

        if(isServer)
            hideClientControls();
        else
            hideServerControls();
    }
}


void MainWindow::on_pushButton_clicked()
{
    if (!trybSieciowyUstawiony) {
        QMessageBox::warning(this, "Błąd", "Najpierw wybierz tryb sieciowy.");
        return;
    }

    if (isServer) {
        if(!server){
            server = new NetworkServer(this);

            connect(server, &NetworkServer::clientDisconnected, this, [this](const QString &addr) {
                for (int i = 0; i < clientListModel->rowCount(); ++i) {
                    QStandardItem *item = clientListModel->item(i);
                    QString displayText = item->text();  // np. "client_1 (192.168.0.2)"
                    if (displayText.contains(addr)) {
                        clientListModel->removeRow(i);
                        break;
                    }
                }

                // Dodatkowo jeśli rozłączony był wyświetlany, to go wyczyść
                if (ui->lbl_Info->text().contains(addr)) {
                    ui->lbl_Info->setText("SERWER ONLINE localhost:" + QString::number(selectedPort));
                    ui->lbl_Info->setStyleSheet("color: green;");
                    this->setWindowTitle("Serwer");
                }
            });

            connect(server, &NetworkServer::clientConnected, this, [this](const QString &clientId, const QString &ip) {
                QString clientDisplayText = clientId + " (" + ip + ")";
                clientListModel->appendRow(new QStandardItem(clientDisplayText));
            });

            if (server->startServer(selectedPort)) {
                connect(server, &NetworkServer::messageReceived, this, &MainWindow::onMessageReceived);
                qDebug() << "Serwer uruchomiony na porcie" << selectedPort;
            } else {
                QMessageBox::critical(this, "Błąd", "Nie można uruchomić serwera.");
            }

            ui->pushButton->setText("Wyłącz serwer");
            ui->lbl_Info->setText("SERWER ONLINE localhost:" + QString::number(selectedPort));
            ui->lbl_Info->setStyleSheet("color: green;");
        }
        else{
            // wyłączamy serwer i rozłączamy klienta
            stopLoop();

            server->stopServer();
            server->deleteLater();
            server = nullptr;

            for (int i = 0; i < clientListModel->rowCount(); ++i) {
                QStandardItem *item = clientListModel->item(i);
                clientListModel->removeRow(i);
            }

            ui->lbl_Info->setText("");
            ui->pushButton->setText("Uruchom serwer");
            ui->lbl_Info->setText("SERWER OFFLINE");
            ui->lbl_Info->setStyleSheet("color: red;");
        }
    }
    else
    {
        if(!client){
            client = new NetworkClient(this);

            connect(client, &NetworkClient::serverDisconnected, this, [this]() {
                ui->pushButton->setText("Połącz z wybranym serwerem");
                ui->lbl_Info->setText("KLIENT niepołączony");
                ui->lbl_Info->setStyleSheet("color: brown;");
            });

            connect(client, &NetworkClient::connected, this, []() {
                qDebug() << "Połączono z serwerem!";
            });

            connect(client, &NetworkClient::messageReceived, this, [this](const QString &msg) {
                double value = msg.toDouble();
                ui->dsb_Dane->setValue(value);
                currentStepValue = value;

                if (isLoopActive) {
                    QTimer::singleShot(100, this, [this]() {
                        performNetworkStep();
                    });
                }
            });

            client->connectToServer(selectedIp, selectedPort);

            // Gdy połączenie zostanie naprawdę nawiązane:
            connect(client, &NetworkClient::connected, this, [this]() {
                ui->pushButton->setText("Rozłącz z serwerem");
                ui->lbl_Info->setText("połączony z " + selectedIp + ":" + QString::number(selectedPort));
                ui->lbl_Info->setStyleSheet("color: cyan;");
            });

            connect(client->getSocket(), &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError socketError){
                stopLoop();
                Q_UNUSED(socketError);
                QString errorMessage = client->getSocket()->errorString();
                QMessageBox::critical(this, "Błąd połączenia", errorMessage);
                client->deleteLater();
                client = nullptr;
            });
        } else{
            stopLoop();

            client->disconnectFromServer();
            client = nullptr;

            ui->pushButton->setText("Połącz z wybranym serwerem");
            ui->lbl_Info->setText("KLIENT niepołączony");
            ui->lbl_Info->setStyleSheet("color: brown;");
        }
    }
}

void MainWindow::onMessageReceived(const QString &msg)
{
    ui->lbl_WynikDane->setText(msg);
}


void MainWindow::hideServerControls()
{
    ui->listView->hide();
    ui->rbtn_taktowanie1->show();
    ui->rbtn_taktowanie2->show();
    ui->label->show();
    ui->dsb_Dane->show();
    ui->pushButton->show();
    ui->pushButton->setText("Połącz z wybranym serwerem");
    ui->lbl_Info->setText("KLIENT niepołączony");
    ui->lbl_Info->setStyleSheet("color: brown;");
    ui->btnStartLoop->show();
}

void MainWindow::hideClientControls()
{
    ui->listView->show();
    ui->lbl_klie->show();
    ui->lbl_Opis->show();
    ui->lbl_WynikDane->show();
    ui->pushButton->show();
    ui->pushButton->setText("Uruchom serwer");
    ui->lbl_Info->setText("SERWER OFFLINE");
    ui->lbl_Info->setStyleSheet("color: red;");
}

void MainWindow::performNetworkStep()
{
    if (!client) {
        return;
    }

    double currentValue = currentStepValue;
    client->sendDataToServer(currentValue);
}

void MainWindow::stopLoop()
{
    // zatrzymaj pętlę
    ui->btnStartLoop->setText("Uruchom pętlę");
    ui->rbtn_taktowanie1->setCheckable(true);
    ui->rbtn_taktowanie2->setCheckable(true);
    isLoopActive = false;
}

void MainWindow::startLoop()
{
    // uruchom pętlę: ustaw flagę i wyślij wartość startową
    double start = ui->dsb_Dane->value();
    client->sendDataToServer(start);

    ui->btnStartLoop->setText("Zatrzymaj pętlę");
    if (!ui->rbtn_taktowanie1->isChecked()) ui->rbtn_taktowanie1->setCheckable(false);
    if (!ui->rbtn_taktowanie2->isChecked()) ui->rbtn_taktowanie2->setCheckable(false);
    isLoopActive = true;
}

void MainWindow::on_btnStartLoop_clicked()
{
    if (!client) {
        QMessageBox::warning(this, "Błąd", "Najpierw połącz klienta z serwerem.");
        return;
    }

    if (isLoopActive) {
        stopLoop();
    } else {
        startLoop();
    }
}

void MainWindow::setActiveClient(int row)
{
    if (row >= 0 && row < clientListModel->rowCount()) {
        QString clientDisplayText = clientListModel->item(row)->text();
        QString clientId = clientDisplayText.section(" ", 0, 0);  // np. client_1

        QString clientIp = clientDisplayText.section("(", 1, 1).chopped(1); // np. 192.168.0.2

        QString infoText = QString("Wybrany klient: %1 (%2)").arg(clientId, clientIp);
        ui->lbl_Info->setText(infoText);
        ui->lbl_Info->setStyleSheet("color: cyan;");
        this->setWindowTitle(clientId);  // zmień tytuł okna

        if (server) {
            server->setActiveClientById(clientId);
        }
    }
}
