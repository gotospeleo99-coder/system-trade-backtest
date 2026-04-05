#ifndef BINARYDBWINDOW_H
#define BINARYDBWINDOW_H

#include <QWidget>
#include "binaryDB.h"


namespace Ui {
class binaryDBwindow;
}

class binaryDBwindow : public QWidget
{
    Q_OBJECT

public:
    explicit binaryDBwindow(QWidget *parent = nullptr);
    ~binaryDBwindow();

private:
    Ui::binaryDBwindow *ui;
private slots:
    void onTestButtonClicked();
};

#endif // BINARYDBWINDOW_H
