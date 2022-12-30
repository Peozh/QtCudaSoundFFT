#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>
#include <QQuickWidget>
#include <QQmlContext>

namespace Ui {
class LoadingDialog;
}

class LoadingDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString percentage READ getPercentage WRITE setPercentage NOTIFY percentageChanged)

public:
    explicit LoadingDialog(QWidget *parent = nullptr);
    ~LoadingDialog();

    QString getPercentage();
    void setPercentage(const QString& newPercentage);
    void setPercentage(int intPercentage);

private:
    Ui::LoadingDialog *ui;
    QQuickWidget* quickWidget = nullptr;
    QString percentage = "0 %";

signals:
    void percentageChanged();
};

#endif // LOADINGDIALOG_H
