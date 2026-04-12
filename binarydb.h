
#ifndef BINARYDB_H
#define BINARYDB_H

#include <QLocale>
#include <QStandardPaths>
#include <QCoreApplication>
#include <filesystem>
#include <vector>
#include <fstream>
#include "struct_util.h"

namespace binaryDB {
int ExportTickerBinary();
int firstExportPricedataBinary();
};
std::string GetFileDate();


class file{
public:
    static bool check();
    static std::vector<std::string> CSVparse(const std::string& line);
    static std::optional<OHLCetc> parseOHLC(const std::vector<std::string>& fields);
    template<typename T>
    bool Savebinary(const std::filesystem::path& path,const std::string& filename,const std::vector<T>* data);
    template<typename T>
    static bool loadBinary(const std::filesystem::path& path, std::vector<T>& records){
        std::ifstream ifs(path,std::ios::binary| std::ios::ate);
        if(!ifs)return false;

        std::streamsize fileSize =ifs.tellg();
        ifs.seekg(0,std::ios::beg);

        records.resize(fileSize / sizeof(T));
        ifs.read(reinterpret_cast<char*>(records.data()),fileSize);

        return ifs.good();

    }
};

class List_stocks{//バイナリー変換前工程用クラス
public:
    static void StockCodeCheck(std::vector<StockCodeID>& stockCode,std::filesystem::path path);//対象コードにIDを埋め込む
    void ListLorder();
    static std::vector<std::string>ListCSVparse(const std::string& line);
};

#endif // BINARYDB_H
