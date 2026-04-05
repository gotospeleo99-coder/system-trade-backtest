#include "binarydbwindow.h"
#include "ui_binarydbwindow.h"
#include <QDebug>

binaryDBwindow::binaryDBwindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::binaryDBwindow)
{
    ui->setupUi(this);
    connect(ui->Test, &QPushButton::clicked, this, &binaryDBwindow::onTestButtonClicked);
}

binaryDBwindow::~binaryDBwindow()
{
    delete ui;
}
void binaryDBwindow::onTestButtonClicked()
{
    int result = binaryDB::ExportTickerBinary();
    if(result == 0){
        qDebug() << "ExportTickerBinary 成功";
    } else {
        qDebug() << "ExportTickerBinary 失敗 result:" << result;
    }
}
