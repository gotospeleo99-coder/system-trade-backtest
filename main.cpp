#include "common.h" // NOLINT
#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <windows.h>
int main(int argc, char *argv[]){


    if(!file::check()){
        //UI実装時にエラーログなどを作ること
        return -1;
    }
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SystemTrade_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
