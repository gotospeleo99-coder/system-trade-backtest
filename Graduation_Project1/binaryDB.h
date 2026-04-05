
#ifndef BINARYDB_H
#define BINARYDB_H

#include <QLocale>
#include <QStandardPaths>
#include <QCoreApplication>
#include <filesystem>
#include "struct_util.h"

namespace binaryDB {
int ExportTickerBinary();

};


class file{
    public:
    static bool check();
        static std::vector<std::string> CSVparse(const std::string& line);
        template<typename T>
        bool Savebinary(const std::filesystem::path& path,const std::string& filename,const std::vector<T>* data);
};
class List_stocks{//バイナリー変換前工程用クラス
public:
    static void StockCodeCheck(std::vector<StockCodeID>& stockCode,std::filesystem::path path);//対象コードにIDを埋め込む
    void ListLorder();
    static std::vector<std::string>CSVparse(const std::string& line);
};
#endif // BINARYDB_H
