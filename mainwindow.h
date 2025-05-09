#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "networkclient.h"
#include "networkserver.h"
#include "roledialog.h"
#include <QMessageBox>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void performNetworkStep();
private slots:
    void on_btn_TrybSieciowy_clicked();
    void on_pushButton_clicked();
    void onMessageReceived(const QString &msg);

    void on_btnStartLoop_clicked();

private:
    Ui::MainWindow *ui;
    NetworkServer *server;
    NetworkClient *client;
    bool isServer;
    QString selectedIp;
    quint16 selectedPort;
    bool trybSieciowyUstawiony = false;

    void hideServerControls();
    void hideClientControls();

    void loop();
    bool isLoopActive = false;
    double value = 0.0;
    double currentStepValue = 0.0;
};
#endif // MAINWINDOW_H
