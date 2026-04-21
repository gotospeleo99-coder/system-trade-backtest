#include "binaryDB.h"
#include "struct_util.h"
#include <filesystem>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <iostream>
#include <QDebug>
#include <unordered_map>

int binaryDB::ExportTickerBinary(){//銘柄確認
    std::vector<StockCodeID> stockCode;
    std::filesystem::path csvroot = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString())/"data"/"csv";
    std::filesystem::path binroot = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString())/"data"/"bin";

    file::StockCodeCheck(stockCode , csvroot);

    file f;
    std::string fillname = "stockList_"+file::GetFileDate()+".bin";
    std::filesystem::path checkpath = binroot/fillname;
    if(!std::filesystem::exists(checkpath)){
        bool result=f.Savebinary(binroot,fillname,&stockCode);
        if(!result){
            qDebug() << "Savebinary failed";
        }
    }


    return 0;
};

int binaryDB::firstExportPricedataBinary(){//価格情報CSVファイルを入れたうえで行う初回バイナリ化
    std::vector<std::vector<OHLCetc>> Pricedate;
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
        file::loadBinary(latest, codeID);
    }
    latest.clear();

    std::unordered_map<std::string, uint16_t> codeMap;
    for (const StockCodeID& rec : codeID) {
        codeMap[rec.code] = rec.id;
    }

    Pricedate.resize(codeID.size());//外側のvectorを銘柄分確保
    codeID.clear();        // 要素を全削除（size=0）。capacity は保持される。
    codeID.shrink_to_fit();// capacity を減らすよう要求（多くの実装でメモリ解放される）。
    // 上記は趣味。vector の挙動確認と技術研修目的。

    for (std::vector<OHLCetc>& inner : Pricedate) {//内側を245日*10年分確保
        inner.reserve(2450);
    }


    std::vector<std::filesystem::path> csvFilespath;
    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(csvroot)) {
        const std::filesystem::path& p = entry.path();
        if (p.extension() == ".csv" && p.filename().string().find("equities_bars_daily_") == 0) {
            csvFilespath.push_back(p);
        }
    }
    std::sort(csvFilespath.begin(), csvFilespath.end());
    for(const std::filesystem::path& p : csvFilespath){

        std::ifstream ifs(p);
        std::string line;
        std::getline(ifs, line);

        std::vector<std::string> header = file::ListCSVparse(line);
        std::string code;
        CSVpriceheader hederstruct;
        for(int i = 0;i<header.size();i++){
            if     (header[i]=="Code")       code =i;
            else if(header[i] == "O")        hederstruct.Open = i;
            else if(header[i] == "H")        hederstruct.High = i;
            else if(header[i] == "L")        hederstruct.Low = i;
            else if(header[i] == "C")        hederstruct.Close = i;
            else if(header[i] == "UL")       hederstruct.UL = i;
            else if(header[i] == "LL")       hederstruct.LL = i;
            else if(header[i] == "AdjFactor")hederstruct.AF = i;
        }
        if(hederstruct.Open == -1||hederstruct.High == -1
            ||hederstruct.Low == -1||hederstruct.Close == -1
            ||hederstruct.UL == -1||hederstruct.LL == -1
            ||hederstruct.AF ==-1 ){
            std::cerr<<p<<"のO,H,L,C,UL,LL,AdjFactorのいずれかのヘッダー情報が抜け落ちていますCSVファイルを確認してください"<<std::endl;
            return 1;
        }

    }
    return 0;
};

bool file::check(){
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
void file::StockCodeCheck(std::vector<StockCodeID>& stockCode,std::filesystem::path path){//銘柄コードに対応したindexにIDを埋め込む。　　正常動作確認
    std::filesystem::path Latestfile;
    std::string latestData ="";
    // auto start = std::chrono::high_resolution_clock::now();
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) return;

    std::regex target(R"(equities_master_(\d{6}(\d{2})?)\.csv)");//ファイル名の正規表現日数が8か6かはファイルによる

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

    if (!Latestfile.empty()) {
        std::ifstream ifs(Latestfile);

        if (!ifs.is_open()) {
            //もしLatestfileにパスが入ってなかった場合の処理
            return;
        }

        int id =0;

        std::string line;
        std::getline(ifs, line);
        std::vector<std::string> header = file::ListCSVparse(line);
        int code= -1 ,MktNm = -1;
        int Damageline = 0;
        for(int i = 0 ; i<header.size();i++){//CSVのヘッダー情報の入手
            if(header[i] == "Code")code = i;
            if(header[i] == "MktNm")MktNm = i;
        }
        if(code == -1||MktNm == -1){
            std::cerr<<"code又はMktNmのヘッダーが見つかりませんでした"<<std::endl;
            std::exit(1);
        }
        while (std::getline(ifs, line)) {//CSVの本文のパース
            std::vector<std::string> row = file::ListCSVparse(line);
            if (row.size() <= std::max(code, MktNm)) {//対象の情報の数がヘッダー情報より少なかった場合
                std::cerr << "CSV row format error: " << line << std::endl;
                continue;
            }
            if (row[code].size() < 5) {
                //std::cerr << "Code length error: " <<line+":"<<row[code] << std::endl;
                Damageline++;
                continue;
            }
            if( row[MktNm]=="プライム"||row[MktNm]=="スタンダード"||row[MktNm]=="グロース"){
                StockCodeID tmp;
                memcpy(tmp.code, row[code].c_str(), 5);
                tmp.code[5] = '\0';
                tmp.id = id++;
                stockCode.push_back(tmp);
            }

        }
        if(Damageline!=0)std::cerr<<"break data"<<Damageline<<std::endl;
    }
    // auto end = std::chrono::high_resolution_clock::now();
    // auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

};

std::vector<std::string> file::ListCSVparse(const std::string& line){

    std::vector<std::string> linedata;
    linedata.reserve(13);
    size_t pos= 0 ;

    while(pos<line.size()){
        if(line[pos] =='"'){
            size_t last =line.find('"',pos+1);
            if(last == std::string::npos) throw std::runtime_error("不正なCSV: 閉じ\"がない");
            linedata.push_back(line.substr(pos +1,last-pos-1));
            pos = last+2;
        }
        else{
            size_t last =line.find(',',pos);
            if (last == std::string::npos) last = line.size();
            linedata.push_back(line.substr(pos,last-pos));
            pos = last+1;
        }

    }
    return linedata;
}

template<typename T>
bool file::Savebinary(const std::filesystem::path& path, const std::string& filename, const std::vector<T>* data) {//新規書き込み上書き

    static_assert(std::is_trivially_copyable_v<T>, "Tはトリビアルコピー可能な型でなければなりません。（std::stringやstd::vector等は使用不可）");

    if (!data || data->empty()) return false;

    // ディレクトリが存在しない場合は作成
    if (!std::filesystem::exists(path)) {
        std::error_code ec;
        std::filesystem::create_directories(path, ec);
        if (ec) return false;
    }

    std::filesystem::path fullPath = path / filename;

    // ファイルが存在すれば追記、なければ新規作成
    std::ios::openmode mode = std::ios::binary | std::ios::out;
    if (std::filesystem::exists(fullPath)) {
        mode |= std::ios::app;  // 追記モード
    }

    std::ofstream ofs(fullPath, mode);
    if (!ofs) return false;

    ofs.write(reinterpret_cast<const char*>(data->data()), data->size() * sizeof(T));
    //data->data() — vectorの生ポインタを取得
    //reinterpret_cast<const char*> — それをバイト列として解釈し直す
    //data->size() * sizeof(T) — 要素数×1要素のバイト数＝総バイト数を書き込む

    return ofs.good();
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

void parseOHLC(const std::vector<std::string>& fields, const CSVpriceheader& header, OHLCetc& data){
    if(fields.size() != CSVpriceheader::count) return;
    data.Open  = fields[header.Open ].empty() ? 0 : std::stod (fields[header.Open ]) * OHLCetc::PRICE_SCALE;
    data.High  = fields[header.High ].empty() ? 0 : std::stod (fields[header.High ]) * OHLCetc::PRICE_SCALE;
    data.Low   = fields[header.Low  ].empty() ? 0 : std::stod (fields[header.Low  ]) * OHLCetc::PRICE_SCALE;
    data.Close = fields[header.Close].empty() ? 0 : std::stod (fields[header.Close]) * OHLCetc::PRICE_SCALE;
    data.UL    = fields[header.UL   ].empty() ? 0 : std::stod (fields[header.UL   ]) * OHLCetc::PRICE_SCALE;
    data.LL    = fields[header.LL   ].empty() ? 0 : std::stod (fields[header.LL   ]) * OHLCetc::PRICE_SCALE;
    data.Vo    = fields[header.Vo   ].empty() ? 0 : std::stoul(fields[header.Vo  ]);
    data.AF    = fields[header.AF   ].empty() ? 0 : std::stof (fields[header.AF   ]);
}

/*void DataLoding::CSV_DataLoader(std::span<OHLC> buffer,const std::filesystem::path& files){//staticを外すように
    int count=0;
    int nullCount = 0;
    std::ifstream ifs(files.string());
    std::string line;
    std::getline(ifs, line);//最初1行スキップ
    while (std::getline(ifs,line)) {
        if (static_cast<size_t>(count) >= buffer.size()) break;
        if (line.empty()) {
            nullCount++;
            if (nullCount >= 3) break;
            continue;
        }
        nullCount = 0;


        try{
            std::stringstream ss(line);
            std::string data,openStr,highStr,lowStr,CloseStr;//,volumeStr;//--今は使わないのでコメントアウト--

            std::getline(ss,data,',');
            std::getline(ss,openStr,',');
            std::getline(ss,highStr,',');
            std::getline(ss,lowStr,',');
            std::getline(ss,CloseStr,',');

            int Open = static_cast<int>(std::stod(openStr)*10);
            buffer[count].Open  = Open;
            buffer[count].High  = static_cast<int>(std::stod(highStr)*10)-Open;
            buffer[count].Low   = static_cast<int>(std::stod(lowStr)*10)-Open;
            buffer[count].Close = static_cast<int>(std::stod(CloseStr)*10)-Open;
            count++;
        }
        catch(...){
            continue;
        }
    }
}*/
