#include "binarydbwindow.h"
#include "binaryDB.h"
#include "ui_binarydbwindow.h"
#include <QDebug>

binaryDBwindow::binaryDBwindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::binarydbwindow)
{
    ui->setupUi(this);
    connect(ui->Test, &QPushButton::clicked, this, &binaryDBwindow::onTestButtonClicked);
    connect(ui->second, &QPushButton::clicked, this, &binaryDBwindow::onsecondButtonClicked);
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
    binaryDB::firstExportPricedataBinary();
}

void binaryDBwindow::onsecondButtonClicked()
{
    int result = binaryDB::firstExportPricedataBinary();
    if(result == 0){
        qDebug() << "firstExportPricedataBinary 成功";
    } else {
        qDebug() << "firstExportPricedataBinary 失敗 result:" << result;
    }
    binaryDB::firstExportPricedataBinary();
}