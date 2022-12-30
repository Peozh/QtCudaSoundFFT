#include "loadingdialog.h"
#include "ui_loadingdialog.h"

LoadingDialog::LoadingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadingDialog)
{
    ui->setupUi(this);
    this->quickWidget = ui->quickWidget;

    qmlRegisterUncreatableType<LoadingDialog>("LoadingDialogPackage", 1, 0, "LoadingDialog", "Access to LoadingDialog class");
    QQmlContext* rootContext = this->quickWidget->rootContext();
    rootContext->setContextProperty("_loadingDialog", this);
    ui->quickWidget->setContentsMargins(0, 0, 0, 0);
    this->setWindowFlag(Qt::FramelessWindowHint, true);
    ui->quickWidget->setSource(QUrl(QStringLiteral("qrc:/loadingDialog.qml")));
}

LoadingDialog::~LoadingDialog()
{
    delete ui;
}

QString LoadingDialog::getPercentage()
{
    return this->percentage;
}

void LoadingDialog::setPercentage(const QString &newPercentage)
{
    if (this->percentage == newPercentage) return;
    this->percentage = newPercentage;
    emit this->percentageChanged();
}

void LoadingDialog::setPercentage(int intPercentage)
{
    QString newPercentage = QString::number(intPercentage) + " %";
    this->setPercentage(newPercentage);
}
