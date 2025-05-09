#ifndef ROLEDIALOG_H
#define ROLEDIALOG_H

#include <QDialog>

namespace Ui {
class RoleDialog;
}

class RoleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoleDialog(QWidget *parent = nullptr);
    ~RoleDialog();

    bool isServerSelected() const;
    QString getAddress() const;
    quint16 getPort() const;

private slots:
    void on_roleComboBox_currentIndexChanged(int index);

private:
    Ui::RoleDialog *ui;
};

#endif // ROLEDIALOG_H
