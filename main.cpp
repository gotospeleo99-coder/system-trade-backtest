#include "common.h" // NOLINT
#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <windows.h>
bool check();
int main(int argc, char *argv[]){


    if(!check()){
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

bool check(){
    std::filesystem::path root = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString());
    const std::array<std::filesystem::path, 4> paths = {
        root / "data",
        root / "data" / "bin",
        root / "data" / "csv",
        root / "config"
    };

    for(const std::filesystem::path& p : paths){
        if(std::filesystem::exists(p)){
            if(!std::filesystem::is_directory(p)){
                return false;
            }
        }
        else{
            if(!std::filesystem::create_directory(p)){
                return false;
            }
        }
    }
    return true;
};