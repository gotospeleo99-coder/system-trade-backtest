
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
    std::string GetFileDate();
    int StockCodeCheck(std::vector<StockCodeID>& stockCode,std::filesystem::path path);//対象コードにIDを埋め込む
    void ListCSVparse(const std::string_view& line,std::vector<std::string_view>& data);
    OHLCetc parseOHLC(const std::vector<std::string_view>& fields, const CSVpriceheader& header);
    ThreadResult PareseCSVFiles(
       const std::vector<std::filesystem::path>& csvFilespath,
       int begin,
       int end,
       const std::unordered_map<std::string_view,uint16_t>& codeMap,
        int chunkIndex);
    };

namespace conversion {
    int NumericalconversionInt(std::string_view line);  // int32_t, uint32_t用

    float NumericalconversionFloat(const std::string& line);  // AF用
};

#endif // BINARYDB_H
