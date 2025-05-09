#include "roledialog.h"
#include "ui_roledialog.h"
#include <QRegularExpressionValidator>


RoleDialog::RoleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RoleDialog)
{
    ui->setupUi(this);

    ui->comboBox->addItem("Serwer");
    ui->comboBox->addItem("Odbiornik");

    // Walidator IP
    QRegularExpression ipRegex(R"((\b(?:\d{1,3}\.){3}\d{1,3}\b))");
    QValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);
    ui->lineEdit->setValidator(ipValidator);

    // Ukryj pole adresu jeśli Serwer
    on_roleComboBox_currentIndexChanged(0);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RoleDialog::on_roleComboBox_currentIndexChanged);
}

RoleDialog::~RoleDialog()
{
    delete ui;
}

bool RoleDialog::isServerSelected() const
{
    return ui->comboBox->currentIndex() == 0;
}

QString RoleDialog::getAddress() const
{
    return ui->lineEdit->text();
}

quint16 RoleDialog::getPort() const
{
    return ui->lineEdit_2->text().toUShort();
}

void RoleDialog::on_roleComboBox_currentIndexChanged(int index)
{
    bool isServer = (index == 0);
    ui->lineEdit->setEnabled(!isServer);

    if(isServer){
        ui->lineEdit->setText("localhost");
    }
    else{
        ui->lineEdit->setText("");
    }
}
