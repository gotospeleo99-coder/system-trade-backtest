#include "binaryDB.h"
#include "struct_util.h"
#include "fileio.h"
#include <filesystem>
#include<fstream>
#include <regex>
#include <vector>
#include <string>
#include <iostream>
#include <future>
#include <QDebug>
#include <unordered_map>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>

int binaryDB::ExportTickerBinary(){//銘柄確認

    std::vector<StockCodeID> stockCode;
    std::filesystem::path csvroot = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString())/"data"/"csv";
    std::filesystem::path binroot = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString())/"data"/"bin";
    qDebug() << QString::fromStdString(csvroot.string());
    int result = file::StockCodeCheck(stockCode, csvroot);
    if(result!=0){
        qDebug()<<"StockCodeCheckの返り値は"<<result;;
        return 1;
    }

    std::string fillname = "stockList_"+file::GetFileDate()+".bin";
    std::filesystem::path checkpath = binroot/fillname;
    if(!std::filesystem::exists(checkpath)){
        bool result=BinaryIO::SaveBinary(binroot,fillname,stockCode);
        if(!result){
            qDebug() << "Savebinary failed";
        }
    }


    return 0;
};

int binaryDB::firstExportPricedataBinary(){//価格情報CSVファイルを入れたうえで行う初回バイナリ化
    //対象ファイルと同名ディレクトリで探査を終了しないように後々改善をする
    std::filesystem::path csvroot = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString())/"data"/"csv";
    std::filesystem::path binroot = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString())/"data"/"bin";

    std::filesystem::path latest;
    std::filesystem::file_time_type latestTime;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(binroot)) {
        const std::filesystem::path& tmppath = entry.path();
        if (tmppath.filename().string().find("stockList_") == 0 && tmppath.extension() == ".bin") {
            std::filesystem::file_time_type tmptime = entry.last_write_time();
            if (latest.empty() || tmptime > latestTime) {
                latestTime = tmptime;
                latest = tmppath;
            }
        }
    }
    std::vector<StockCodeID> codeID;

    if (!latest.empty()) {
        BinaryIO::LoadBinary(latest, codeID);
    }
    latest.clear();



    std::unordered_map<std::string_view, uint16_t> codeMap;
    for (const StockCodeID& rec : codeID) {
        codeMap[rec.code] = rec.id;
    }

    std::vector<std::filesystem::path> csvFilespath;
    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(csvroot)) {
        const std::filesystem::path& p = entry.path();
        if (p.extension() == ".csv" && p.filename().string().find("equities_bars_daily_") == 0) {
            csvFilespath.push_back(p);
        }
    }
    std::sort(csvFilespath.begin(), csvFilespath.end());

    unsigned int cores = std::thread::hardware_concurrency();
    if(cores>1)--cores;
    else cores=1;

    int chunkcount=0;
    int base = csvFilespath.size() / cores;
    int amari = csvFilespath.size() % cores-1;
    std::vector<std::future<ThreadResult>> futures;
    futures.reserve(cores);

    size_t begin = 0;
    for(int i = 0; i < cores; i++){
        size_t chunk = base + (i < amari ? 1 : 0);
        size_t end = begin + chunk;
        futures.emplace_back(
            std::async(std::launch::async,file::PareseCSVFiles,std::cref(csvFilespath),begin,end,std::cref(codeMap),i));
        begin = end;
    }

    std::vector<ThreadResult> results;
    results.reserve(cores);

    for(auto& f : futures){
        results.push_back(f.get());
    }

    std::sort(results.begin(), results.end(),
              [](const ThreadResult& a, const ThreadResult& b){
                  return a.fileIndex < b.fileIndex;
              });

    ThreadResult merged;
    merged.pricedata.resize(codeMap.size());
    merged.fileIndex=0;

    for(ThreadResult& r : results){
        // timeline
        merged.timeline.insert(
            merged.timeline.end(),
            std::make_move_iterator(r.timeline.begin()),
            std::make_move_iterator(r.timeline.end())
            );

        // 各銘柄
        for(size_t id = 0; id < r.pricedata.size(); id++){
            merged.pricedata[id].insert(
                merged.pricedata[id].end(),
                std::make_move_iterator(r.pricedata[id].begin()),
                std::make_move_iterator(r.pricedata[id].end())
                );
        }
    }
    BinaryIO::SaveBinary(binroot,"timeline.bin",merged.timeline);
    std::string filename;
    for(int i = 0;i < merged.pricedata.size();i++){
        filename.clear();
        filename = std::string(codeID[i].code)+"bin";
        BinaryIO::SaveBinary(binroot,filename,merged.pricedata[i]);

    }

    return 0;
};

int file::StockCodeCheck(std::vector<StockCodeID>& stockCode,std::filesystem::path path){//銘柄コードに対応したindexにIDを埋め込む。　　正常動作確認
    std::filesystem::path Latestfile;
    std::string latestData ="";
    // auto start = std::chrono::high_resolution_clock::now();
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) return 1;

    std::regex target(R"(equities_master_(\d{6}(\d{2})?)\.csv)");//ファイル名の正規表現日数が8か6かはファイルによる
    //対象ファイルと同名ディレクトリで探査を終了しないように後々改善をする
    for(const std::filesystem::directory_entry& entry: std::filesystem::recursive_directory_iterator(path)) {
        std::string filename = entry.path().filename().string();
        std::smatch match;
        if(std::regex_match(filename,match,target)){
            if(match[1].str()>latestData){
                latestData = match[1].str();
                Latestfile = entry.path();
            }
        }
    }
    //file::ConvertAllCSVtoUTF8(Latestfile);

    if (!Latestfile.empty()) {
        std::ifstream ifs(Latestfile);
        qDebug()<< "==== DEBUG CHECK START ====\n";
        qDebug()<<QString::fromStdString( Latestfile.string());
        qDebug()<< "STRING SIZE = " << Latestfile.string().size() << "\n";
        qDebug()<< "exists = " << std::filesystem::exists(Latestfile) << "\n";
        qDebug()<< "is_regular_file = " << std::filesystem::is_regular_file(Latestfile) << "\n";

        if (!ifs.is_open()) {
            //もしLatestfileにパスが入ってなかった場合の処理
            return 2;
        }
        qDebug()<<"fileオープン";
        int id =0;

        std::string line;
        std::getline(ifs, line);
        std::vector<std::string_view> header;
        file::ListCSVparse(line,header);
        int code= -1 ,Mkt = -1;
        int Damageline = 0;
        for(int i = 0 ; i<header.size();i++){//CSVのヘッダー情報の入手
            if(header[i] == "Code")code = i;
            if(header[i] == "Mkt")Mkt = i;
        }
        if(code == -1||Mkt == -1){
            std::cerr<<"code又はMktNmのヘッダーが見つかりませんでした"<<std::endl;
            std::exit(1);
        }
        for(int i = 0;i<header.size();i++){
            qDebug()<<header[i];
        }
        while (std::getline(ifs, line)) {//CSVの本文のパース
            std::vector<std::string_view> row;
            file::ListCSVparse(line,row);
            for(int i = 0;i<row.size();i++){
                qDebug()<<"row["<<i<<"]"<<row[i];
            }
            if (row.size() <= std::max(code, Mkt)) {//対象の情報の数がヘッダー情報より少なかった場合
                std::cerr << "CSV row format error: " << line << std::endl;
                continue;
            }
            if (row[code].size() < 5) {
                //std::cerr << "Code length error: " <<line+":"<<row[code] << std::endl;
                Damageline++;
                continue;
            }
            if( targetMkt.count(std::stoi(std::string(row[Mkt])))){
                StockCodeID tmp;
                memcpy(tmp.code, row[code].data(), 5);
                tmp.code[5] = '\0';
                tmp.id = id++;
                stockCode.push_back(tmp);
            }

        }
        if(Damageline!=0)std::cerr<<"break data"<<Damageline<<std::endl;
    }
    // auto end = std::chrono::high_resolution_clock::now();
    // auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    return 0;
};

void file::ListCSVparse(const std::string_view& line,std::vector<std::string_view>& data){//dataはあらかじめ外でriserveするように確保してください
    //findを使わない形に修正することを検討

    size_t pos = 0;

    while(pos < line.size()){
        if(line[pos] == '"'){
            size_t last = line.find('"', pos + 1);
            if(last == std::string::npos){
                throw std::runtime_error("不正なCSV");
            }
            data.emplace_back(line.data() + pos + 1, last - pos - 1);
            pos = last + 2;
        }
        else{
            size_t last = line.find(',', pos);
            if(last == std::string::npos){
                last = line.size();
            }
            data.emplace_back(line.data() + pos, last - pos);
            pos = last + 1;
        }
    }
}



std::string file::GetFileDate(){
    std::time_t now = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &now);

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << (tm.tm_year + 1900)
        << std::setw(2) << std::setfill('0') << (tm.tm_mon + 1)
        << std::setw(2) << std::setfill('0') << tm.tm_mday;

    return oss.str();
}

OHLCetc file::parseOHLC(const std::vector<std::string_view>& fields, const CSVpriceheader& header){
    OHLCetc data;
    data.Open  = fields[header.Open ].empty() ? 0 : conversion::NumericalconversionInt(fields[header.Open ]);
    data.High  = fields[header.High ].empty() ? 0 : conversion::NumericalconversionInt(fields[header.High ]);
    data.Low   = fields[header.Low  ].empty() ? 0 : conversion::NumericalconversionInt(fields[header.Low  ]);
    data.Close = fields[header.Close].empty() ? 0 : conversion::NumericalconversionInt(fields[header.Close]);
    // data.UL    = fields[header.UL   ].empty() ? 0 : conversion::NumericalconversionInt(fields[header.UL   ]);
    // data.LL    = fields[header.LL   ].empty() ? 0 : conversion::NumericalconversionInt(fields[header.LL   ]);
    data.Vo    = fields[header.Vo   ].empty() ? 0 : conversion::NumericalconversionInt(fields[header.Vo   ]);
    data.AF    = fields[header.AF   ].empty() ? 0 : std::stof(std::string(fields[header.AF]));
    return data;
}

int conversion::NumericalconversionInt(const std::string_view line){//関数名とnamespace含めたら長い名前の変更は検討しよう（コードが若干読みにくい以外の実害はない）//
    int value = 0;
    bool Periodjudgment=false;
    for(char tmp:line){
        if(tmp=='.'){
            Periodjudgment =true;
            continue;
        }
        value = value * 10+(tmp-'0');
    }
    if(!Periodjudgment)value*=10;
    return value;
}

ThreadResult file::PareseCSVFiles(
    const std::vector<std::filesystem::path>& csvFilespath,
    int begin,
    int end,
    const std::unordered_map<std::string_view,uint16_t>& codeMap,
    int chunkIndex){

    ThreadResult data;
    data.fileIndex=chunkIndex;
    data.pricedata.resize(codeMap.size());
    int vectorsize = (end - begin)*30;
    for(int i=0;i<codeMap.size();i++){
        data.pricedata[i].reserve(vectorsize);
    }
    for(int i = begin; i < end; ++i){
        const std::filesystem::path& p = csvFilespath[i];
        std::ifstream ifs(p);
        if (!ifs) {
            std::cerr << "file open failed: " << p << std::endl;
            exit(1);
        }
        std::string line;
        line.reserve(256);
        std::getline(ifs, line);
        std::vector<std::string_view> tmpdata;
        file::ListCSVparse(line,tmpdata);
        std::string code;
        CSVpriceheader hederstruct;
        for(int i = 0;i<tmpdata.size();i++){
            if     (tmpdata[i] =="Date")      hederstruct.date=i;
            else if(tmpdata[i]=="Code")       hederstruct.code =i;
            else if(tmpdata[i] == "O")        hederstruct.Open = i;
            else if(tmpdata[i] == "H")        hederstruct.High = i;
            else if(tmpdata[i] == "L")        hederstruct.Low = i;
            else if(tmpdata[i] == "C")        hederstruct.Close = i;
            else if(tmpdata[i] == "Vo")       hederstruct.Vo = i;
            else if(tmpdata[i] == "AdjFactor")hederstruct.AF = i;
        }
        if(hederstruct.Open == -1||hederstruct.High == -1
            ||hederstruct.Low == -1||hederstruct.Close == -1
            ||hederstruct.AF ==-1 ||hederstruct.code == -1
            ||hederstruct.date ==-1||hederstruct.Vo ==-1){
            qDebug()<<p.u8string()<<"のO,H,L,C,UL,LL,AdjFactorのいずれかのヘッダー情報が抜け落ちていますCSVファイルを確認してください"<<tmpdata.size()<<line;
            for(std::string_view s :tmpdata){
                qDebug()<<s;
            }
            exit(2);
        }

        std::string datecomparison;//日付比較用//
        int count=-1;


        while (std::getline(ifs, line)){//そもそもgetline使うのをやめることを考えるべき実装が楽だから使ってはいるが
            tmpdata.clear();
            file::ListCSVparse(line,tmpdata);

            if (tmpdata.size() < CSVpriceheader::count) {//保険の破損対策発生した場合データの整合性が取れなくなるので即時終了
                std::cerr << "CSV broken: column count mismatch at line: " << line << std::endl;
                exit(3);
            }

            std::unordered_map<std::string_view, uint16_t>::const_iterator it =
                codeMap.find(tmpdata[hederstruct.code]);//毎回一時オブジェクト

            if (it == codeMap.end()) continue;

            if(datecomparison != tmpdata[hederstruct.date]){
                std::string dateStr(tmpdata[hederstruct.date]);
                dateStr.erase(std::remove(dateStr.begin(), dateStr.end(), '-'), dateStr.end());
                data.timeline.push_back(std::stoi(dateStr));
                datecomparison = tmpdata[hederstruct.date];
                count++;
            }

            uint16_t id = it->second;
            if (tmpdata.size() < CSVpriceheader::count) tmpdata.resize(CSVpriceheader::count);

            data.pricedata[id].push_back(file::parseOHLC(tmpdata, hederstruct));
        }
    }
    return data;
}

