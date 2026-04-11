#ifndef BINARYDBWINDOW_H
#define BINARYDBWINDOW_H

#include <QWidget>
#include "binaryDB.h"


namespace Ui {
class binarydbwindow;
}

class binaryDBwindow : public QWidget
{
    Q_OBJECT

public:
    explicit binaryDBwindow(QWidget *parent = nullptr);
    ~binaryDBwindow();

private:
    Ui::binarydbwindow *ui;
private slots:
    void onTestButtonClicked();
};

#endif // BINARYDBWINDOW_H
