
#ifndef BINARYDB_H
#define BINARYDB_H

#include <QLocale>
#include <QStandardPaths>
#include <QCoreApplication>
#include <filesystem>
#include <vector>
#include "struct_util.h"

namespace binaryDB {
int ExportTickerBinary();
int firstExportPricedataBinary();
};

namespace BinaryIO{
template<typename T>
bool SaveBinary(const std::filesystem::path& path,const std::string& filename,const std::vector<T>& data);
template<typename T>
bool LoadBinary(const std::filesystem::path& path, std::vector<T>& records);
};


namespace file{
    //void ConvertAllCSVtoUTF8(const std::filesystem::path& csvroot);
    std::vector<std::string> CSVparse(const std::string& line);
    void parseOHLC(const std::vector<std::string>& fields,const CSVpriceheader& header, OHLCetc& data,int& date);
    std::string GetFileDate();
    int StockCodeCheck(std::vector<StockCodeID>& stockCode,std::filesystem::path path);//対象コードにIDを埋め込む
    std::vector<std::string>ListCSVparse(const std::string& line);
    bool InsertPriceRecord(std::filesystem::path p,OHLCetc data, CSVpriceheader hedear,int );
    bool parseOHLC(const std::vector<std::string>& fields, const CSVpriceheader& header, OHLCetc& data);
    inline const std::string& safe_field(const std::vector<std::string>& v, int idx);
};


#endif // BINARYDB_H
