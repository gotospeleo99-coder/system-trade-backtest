
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



class file{
public:
    static bool check();
    static std::vector<std::string> CSVparse(const std::string& line);
    static void parseOHLC(const std::vector<std::string>& fields,const CSVpriceheader& header, OHLCetc& data,int& date);
    static std::string GetFileDate();
    static void StockCodeCheck(std::vector<StockCodeID>& stockCode,std::filesystem::path path);//対象コードにIDを埋め込む
    static std::vector<std::string>ListCSVparse(const std::string& line);
    static bool InsertPriceRecord(std::filesystem::path p,OHLCetc data, CSVpriceheader hedear,int );
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


#endif // BINARYDB_H
