#ifndef FILEIO_H
#define FILEIO_H
#include <filesystem>
#include<vector>
#include<fstream>

namespace BinaryIO{
template<typename T>
bool LoadBinary(const std::filesystem::path& path, std::vector<T>& records){
    std::ifstream ifs(path,std::ios::binary| std::ios::ate);
    if(!ifs)return false;

    std::streamsize fileSize =ifs.tellg();
    ifs.seekg(0,std::ios::beg);

    records.resize(fileSize / sizeof(T));
    ifs.read(reinterpret_cast<char*>(records.data()),fileSize);

    return ifs.good();

}
template<typename T>
bool SaveBinary(const std::filesystem::path& path, const std::string& filename, const std::vector<T>& data) {//新規書き込み上書き

    static_assert(std::is_trivially_copyable_v<T>, "Tはトリビアルコピー可能な型でなければなりません。（std::stringやstd::vector等は使用不可）");

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

    ofs.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
    //data->data() — vectorの生ポインタを取得
    //reinterpret_cast<const char*> — それをバイト列として解釈し直す
    //data->size() * sizeof(T) — 要素数×1要素のバイト数＝総バイト数を書き込む

    return ofs.good();
}

};

#endif // FILEIO_H
